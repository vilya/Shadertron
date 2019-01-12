// Copyright 2019 Vilya Harvey
#include "Shader.h"

#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QMessageLogger>
#include <QRegularExpression>

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace vh {

  //
  // Private helper methods
  //

  static QString preprocessShaderSource(const QString& filename, const QMap<QString, QString>& macros)
  {
    QFileInfo fileInfo(filename);
    if (!fileInfo.exists()) {
      return "";
    }

    // Replace all #macro lines with their replacement text.
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
      return "";
    }

    QString code = QString::fromLocal8Bit(file.readAll());
    file.close();

    QMapIterator<QString, QString> it(macros);
    while (it.hasNext()) {
      it.next();
      QString before = QString("#macro %1").arg(it.key());
      QString after = it.value();
      code.replace(before, after);
    }

    // Remove any remaining unmatched #macro lines.
    QRegularExpression macroRE("#macro.*$", QRegularExpression::MultilineOption);
    code.replace(macroRE, "");

  #if 0
    qDebug("Shader file %s after preprocessing:", qPrintable(filename));
    qDebug("-----");
    qDebug("%s", qPrintable(code));
    qDebug("-----");
    QMapIterator<QString, QString> it2(macros);
    while (it2.hasNext()) {
      it2.next();
      qDebug("macro %s:", qPrintable(it2.key()));
      qDebug("%s", qPrintable(it2.value()));
    }
  #endif

    return code;
  }


  static int findPassByName(const QString& name)
  {
    if (name == "Image") {
      return Shader::kPass_Final;
    }
    else if (name == "Buffer A") {
      return Shader::kPass_A;
    }
    else if (name == "Buffer B") {
      return Shader::kPass_B;
    }
    else if (name == "Buffer C") {
      return Shader::kPass_C;
    }
    else if (name == "Buffer D") {
      return Shader::kPass_D;
    }
    else {
      return -1;
    }
  }


  static bool loadJSONString(const QJsonObject& json, const char* fieldName, QString& dst)
  {
    if (!json.contains(fieldName)) {
      return false;
    }
    QJsonValue val = json[fieldName];
    if (!val.isString()) {
      return false;
    }
    dst = val.toString();
    return true;
  }


  static bool loadJSONInt(const QJsonObject& json, const char* fieldName, int& dst)
  {
    if (!json.contains(fieldName)) {
      return false;
    }
    QJsonValue val = json[fieldName];
    if (!val.isDouble()) {
      return false;
    }
    dst = static_cast<int>(val.toDouble());
    return true;
  }


  static bool loadJSONBool(const QJsonObject& json, const char* fieldName, bool& dst)
  {
    if (!json.contains(fieldName)) {
      return false;
    }
    QJsonValue val = json[fieldName];
    if (val.isBool()) {
      dst = val.toBool();
      return true;
    }
    else if (val.isDouble()) {
      int tmp = static_cast<int>(val.toDouble());
      if (tmp < 0 || tmp > 1) {
        return false;
      }
      dst = (tmp != 0);
      return true;
    }
    else if (val.isString()) {
      QString tmp = val.toString();
      if (tmp == "true") {
        dst = true;
        return true;
      }
      else if (tmp == "false") {
        dst = false;
        return true;
      }
      else {
        return false;
      }
    }
    return false;
  }


  static bool loadJSONStringList(const QJsonObject& json, const char* fieldName, QStringList& dst)
  {
    if (!json.contains(fieldName)) {
      return false;
    }
    QJsonValue val = json[fieldName];
    if (!val.isArray()) {
      return false;
    }
    QJsonArray arr = val.toArray();
    for (int i = 0; i < arr.size(); i++) {
      if (!arr[i].isString()) {
        return false;
      }
    }
    for (int i = 0; i < arr.size(); i++) {
      dst.push_back(arr[i].toString());
    }
    return true;
  }


  //
  // ShaderInfo public methods
  //

  void ShaderInfo::loadJSON(const QJsonObject& json)
  {
    bool ok = loadJSONString(json, "id",          id) &&
              loadJSONString(json, "date",        date) &&
              loadJSONInt   (json, "viewed",      viewed) &&
              loadJSONString(json, "name",        name) &&
              loadJSONString(json, "username",    username) &&
              loadJSONString(json, "description", description) &&
              loadJSONInt   (json, "likes",       likes) &&
              loadJSONInt   (json, "published",   published) &&
              loadJSONInt   (json, "flags",       flags) &&
              loadJSONStringList(json, "tags",    tags) &&
              loadJSONBool  (json, "hasliked",    hasLiked);
    if (!ok) {
      throw std::runtime_error("JSON contained invalid or incomplete ShaderInfo data");
    }
  }


  //
  // Sampler public methods
  //

  void Sampler::loadJSON(const QJsonObject& json)
  {
    bool ok = loadJSONString(json, "filter",   filter) &&
              loadJSONString(json, "wrap",     wrap) &&
              loadJSONBool  (json, "vflip",    vflip) &&
              loadJSONBool  (json, "srgb",     srgb) &&
              loadJSONString(json, "internal", internal);
    if (!ok) {
      throw std::runtime_error("JSON contained invalid or incomplete Sampler data");
    }
  }


  //
  // RenderPassInput public methods
  //

  void RenderPassInput::loadJSON(const QJsonObject& json)
  {
    bool ok = loadJSONInt   (json, "id",        id) &&
              loadJSONString(json, "src",       src) &&
              loadJSONInt   (json, "channel",   channel) &&
              loadJSONBool  (json, "published", published);
    if (!ok) {
      throw std::runtime_error("JSON contained invalid or incomplete RenderPassInput data");
    }

    QString ctype;
    ok = loadJSONString(json, "ctype", ctype);
    if (!ok) {
      throw std::runtime_error("RenderPassInput did not have a valid ctype field");
    }
    if (ctype == "image") {
      type = RenderPassInputType::eTexture;
    }
    else if (ctype == "buffer") {
      type = RenderPassInputType::eRenderPass;
    }
    else {
      throw std::runtime_error("RenderPassInput has an unrecognised ctype value");
    }

    ok = json.contains("sampler") && json["sampler"].isObject();
    if (!ok) {
      throw std::runtime_error("JSON contained invalid or incomplete RenderPassInput data");
    }
    sampler.loadJSON(json["sampler"].toObject());
  }


  //
  // RenderPassOutput public methods
  //

  void RenderPassOutput::loadJSON(const QJsonObject& json)
  {
    bool ok = loadJSONInt(json, "id", id) &&
              loadJSONInt(json, "channel", channel);
    if (!ok) {
      throw std::runtime_error("JSON contained invalid or incomplete RenderPassOutput data");
    }
  }


  //
  // RenderPass public methods
  //

  void RenderPass::loadJSON(const QJsonObject &json, const QDir& refDir)
  {
    bool ok = loadJSONString(json, "name", name) &&
              loadJSONString(json, "description", description) &&
              loadJSONString(json, "type", type);
    if (!ok) {
      throw std::runtime_error("JSON contained invalid or incomplete RenderPass data");
    }

    if (loadJSONString(json, "filename", filename) && !filename.isEmpty()) {
      QFileInfo fileInfo(filename);
      if (fileInfo.isRelative()) {
        filename = refDir.absoluteFilePath(filename);
      }
      QFile file(filename);
      if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Unable to open external file");
      }
      code = QString::fromLocal8Bit(file.readAll());
      file.close();
    }
    else if (!loadJSONString(json, "code", code)) {
      throw std::runtime_error("renderpass has no external file or inline source code");
    }

    ok = json.contains("inputs") && json["inputs"].isArray() &&
         json.contains("outputs") && json["outputs"].isArray();
    if (!ok) {
      throw std::runtime_error("JSON contained invalid or incomplete data for RenderPass inputs or outputs");
    }

    QJsonArray inputArr = json["inputs"].toArray();
    if (inputArr.size() > kMaxInputs) {
      throw std::runtime_error("JSON contained a render pass input array which was larger than the permitted maximum");
    }
    for (int i = 0; i < inputArr.size(); i++) {
      if (!inputArr[i].isObject()) {
        throw std::runtime_error("JSON contained invalid render pass input data: not all inputs are JSON objects");
      }
    }
    for (int i = 0; i < inputArr.size(); i++) {
      inputs[i].loadJSON(inputArr[i].toObject());
    }

    QJsonArray outputArr = json["outputs"].toArray();
    if (outputArr.size() > 1 || (outputArr.size() == 1 && !outputArr[0].isObject())) {
      throw std::runtime_error("JSON contains invalid render pass output data: the output array had more than one element");
    }
    if (outputArr.size() != 0) {
      output.loadJSON(outputArr[0].toObject());
    }
  }


  //
  // Project public methods
  //

  Shader::Shader(QObject *parent) :
    QObject(parent),
    _fileWatcher()
  {
    _fileWatcher.setParent(this);
    QObject::connect(&_fileWatcher, &QFileSystemWatcher::fileChanged, this, &Shader::handleSourceFileChange);
  }


  Shader::Shader(const QString& filename, QObject* parent) :
    QObject(parent),
    _fileWatcher()
  {
    _fileWatcher.setParent(this);
    QObject::connect(&_fileWatcher, &QFileSystemWatcher::fileChanged, this, &Shader::handleSourceFileChange);

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
      throw std::runtime_error("failed to open shader file");
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (doc.isEmpty() || doc.isNull() || !doc.isObject()) {
      throw std::runtime_error("document root is not a JSON object");
    }

    QJsonObject json = doc.object();
    bool ok = json.contains("Shader") && json["Shader"].isObject();
    if (!ok) {
      throw std::runtime_error("top-level Shader field is missing or not a JSON object");
    }

    QFileInfo fileInfo(filename);
    loadJSON(json["Shader"].toObject(), fileInfo.absoluteDir());

    for (int i = 0; i < kMaxPasses; i++) {
      if (hasPass(i) && !_renderpasses[i].filename.isEmpty()) {
        _fileWatcher.addPath(_renderpasses[i].filename);
      }
    }
    if (!_commonSourceFile.isEmpty()) {
      _fileWatcher.addPath(_commonSourceFile);
    }

    qDebug("Common source code is:");
    qDebug("%s", qPrintable(_commonSourceCode));
  }


  bool Shader::hasPass(int passIdx) const
  {
    return !_renderpasses[passIdx].type.isEmpty();
  }


  QString Shader::passSource(int passIdx) const
  {
    if (!hasPass(passIdx)) {
      return "";
    }

    QMap<QString, QString> macros;
    macros["SAMPLER_0_TYPE"] = "#define SAMPLER_0_TYPE sampler2D";
    macros["SAMPLER_1_TYPE"] = "#define SAMPLER_1_TYPE sampler2D";
    macros["SAMPLER_2_TYPE"] = "#define SAMPLER_2_TYPE sampler2D";
    macros["SAMPLER_3_TYPE"] = "#define SAMPLER_3_TYPE sampler2D";
    macros["COMMON_CODE"] = _commonSourceCode;
    macros["USER_CODE"] = _renderpasses[passIdx].code;

    return preprocessShaderSource(":/glsl/template.frag", macros);
  }


  bool Shader::hasPassInput(int passIdx, int inputIdx) const
  {
    return hasPass(passIdx) && (_renderpasses[passIdx].inputs[inputIdx].type != RenderPassInputType::eNone);
  }


  RenderPassInputType Shader::inputType(int passIdx, int inputIdx) const
  {
    assert(hasPassInput(passIdx, inputIdx));
    return _renderpasses[passIdx].inputs[inputIdx].type;
  }


  QImage Shader::inputImage(int passIdx, int inputIdx) const
  {
    assert(hasPassInput(passIdx, inputIdx));
    assert(inputType(passIdx, inputIdx) == RenderPassInputType::eTexture);

    const RenderPassInput& input = _renderpasses[passIdx].inputs[inputIdx];
    QImage img(input.src);
    if (img.isNull()) {
      qCritical("Unable to load image %s for render pass %d, input %d",
                qPrintable(input.src), passIdx, inputIdx);
      return img;
    }

    // Qt loads images upside-down relative to OpenGL, so the common case is to
    // flip them.
    if (!input.sampler.vflip) {
      img = img.mirrored();
    }

    return img;
  }


  //
  // Shader private slots
  //

  void Shader::handleSourceFileChange(const QString& filename)
  {
    qDebug("Detected a change to %s", qPrintable(filename));

    QString newCode = "";

    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
      newCode = QString::fromLocal8Bit(file.readAll());
      file.close();
    }

    if (filename == _commonSourceFile) {
      _commonSourceCode = newCode;
      emit commonSourceChanged();
    }

    for (int i = 0; i < kMaxPasses; i++) {
      if (hasPass(i) && _renderpasses[i].filename == filename) {
        _renderpasses[i].code = newCode;
        emit passSourceChanged(i);
      }
    }
  }


  //
  // Shader private methods
  //

  void Shader::loadJSON(const QJsonObject& json, const QDir& refDir)
  {
    bool ok = loadJSONString(json, "ver", _ver);
    if (!ok) {
      throw std::runtime_error("JSON missing version section");
    }

    ok = json.contains("info") && json["info"].isObject();
    if (!ok) {
      throw std::runtime_error("JSON missing shader info section");
    }
    _shaderInfo.loadJSON(json["info"].toObject());

    ok = json.contains("renderpass") && json["renderpass"].isArray();
    if (!ok) {
      throw std::runtime_error("JSON missing renderpass array");
    }
    QJsonArray arr = json["renderpass"].toArray();
    for (int i = 0; i < arr.size(); i++) {
      RenderPass tmp;
      tmp.loadJSON(arr[i].toObject(), refDir);

      if (tmp.type == "common") {
        _commonSourceCode = tmp.code;
        _commonSourceFile = tmp.filename;
      }
      else {
        int passIdx = findPassByName(tmp.name);
        if (passIdx == -1) {
          throw std::runtime_error("Unknown render pass name");
        }
        _renderpasses[passIdx] = tmp;
      }
    }
  }

} // namespace vh
