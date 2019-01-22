// Copyright 2019 Vilya Harvey
#include "ShaderToy.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMap>
#include <QMessageLogger>

#include <stdexcept>

namespace vh {

  //
  // ShaderToySampler public methods
  //

  void ShaderToySampler::fromJSON(const QJsonObject& json)
  {
    filter   = json["filter"].toString();
    wrap     = json["wrap"].toString();
    vflip    = json["vflip"].toString();
    srgb     = json["srgb"].toString();
    internal = json["internal"].toString();
  }


  QJsonObject ShaderToySampler::toJSON() const
  {
    QJsonObject json;
    json["filter"]   = filter;
    json["wrap"]     = wrap;
    json["vflip"]    = vflip;
    json["srgb"]     = srgb;
    json["internal"] = internal;
    return json;
  }


  bool ShaderToySampler::isValid() const
  {
    // TODO: add validation logic for a ShaderToySampler
    return true;
  }


  //
  // ShaderToyInput public methods
  //

  void ShaderToyInput::fromJSON(const QJsonObject& json)
  {
    id        = static_cast<int>(json["id"].toDouble(-1.0));
    src       = json["src"].toString();
    ctype     = json["ctype"].toString();
    channel   = static_cast<int>(json["channel"].toDouble(-1.0));
    published = static_cast<int>(json["published"].toDouble(-1.0));

    sampler.fromJSON(json["sampler"].toObject());
  }


  QJsonObject ShaderToyInput::toJSON() const
  {
    QJsonObject json;
    json["id"]        = id;
    json["src"]       = src;
    json["ctype"]     = ctype;
    json["channel"]   = channel;
    json["sampler"]   = sampler.toJSON();
    json["published"] = published;
    return json;
  }


  bool ShaderToyInput::isValid() const
  {
    // TODO: validate other fields
    if (!sampler.isValid()) {
      return false;
    }

    return true;
  }


  //
  // ShaderToyOutput public methods
  //

  void ShaderToyOutput::fromJSON(const QJsonObject& json)
  {
    id      = static_cast<int>(json["id"].toDouble(-1.0));
    channel = static_cast<int>(json["channel"].toDouble(-1.0));
  }


  QJsonObject ShaderToyOutput::toJSON() const
  {
    QJsonObject json;
    json["id"]      = id;
    json["channel"] = channel;
    return json;
  }


  bool ShaderToyOutput::isValid() const
  {
    // TODO: add validation logic for a ShaderToyOutput
    return true;
  }


  //
  // ShaderToyInfo public methods
  //

  void ShaderToyInfo::fromJSON(const QJsonObject& json)
  {
    id          = json["id"].toString();
    date        = json["date"].toString();
    viewed      = static_cast<int>(json["viewed"].toDouble(-1.0));
    name        = json["name"].toString();
    username    = json["username"].toString();
    description = json["description"].toString();
    likes       = static_cast<int>(json["likes"].toDouble(-1.0));
    published   = static_cast<int>(json["published"].toDouble(-1.0));
    flags       = static_cast<int>(json["flags"].toDouble(-1.0));
    hasliked    = static_cast<int>(json["hasliked"].toDouble(-1.0));

    QJsonArray jsonTags = json["tags"].toArray();
    tags.clear();
    for (int i = 0; i < jsonTags.size(); i++) {
      tags.push_back(jsonTags[i].toString());
    }
  }


  QJsonObject ShaderToyInfo::toJSON() const
  {
    QJsonObject json;

    json["id"]          = id;
    json["date"]        = date;
    json["viewed"]      = viewed;
    json["name"]        = name;
    json["username"]    = username;
    json["description"] = description;
    json["likes"]       = likes;
    json["published"]   = published;
    json["flags"]       = flags;
    json["hasliked"]    = hasliked;

    QJsonArray jsonTags;
    for (int i = 0; i < tags.size(); i++) {
      jsonTags.push_back(tags[i]);
    }
    json["tags"] = jsonTags;

    return json;
  }


  bool ShaderToyInfo::isValid() const
  {
    // TODO: add validation logic for ShaderToyInfo
    return true;
  }


  //
  // ShaderToyRenderPass public methods
  //

  void ShaderToyRenderPass::fromJSON(const QJsonObject& json)
  {
    code        = json["code"].toString();
    name        = json["name"].toString();
    description = json["description"].toString();
    type        = json["type"].toString();

    filename    = json["filename"].toString();

    QJsonArray jsonInputs = json["inputs"].toArray();
    inputs.clear();
    for (int i = 0; i < jsonInputs.size(); i++) {
      ShaderToyInput input;
      input.fromJSON(jsonInputs[i].toObject());
      inputs.push_back(input);
    }

    QJsonArray jsonOutputs = json["outputs"].toArray();
    outputs.clear();
    for (int i = 0; i < jsonOutputs.size(); i++) {
      ShaderToyOutput output;
      output.fromJSON(jsonOutputs[i].toObject());
      outputs.push_back(output);
    }
  }


  QJsonObject ShaderToyRenderPass::toJSON() const
  {
    QJsonObject json;

    json["code"]        = code;
    json["name"]        = name;
    json["description"] = description;
    json["type"]        = type;

    json["filename"]    = filename;

    QJsonArray jsonInputs;
    for (int i = 0; i < inputs.size(); i++) {
      jsonInputs.push_back(inputs[i].toJSON());
    }
    json["inputs"] = jsonInputs;

    QJsonArray jsonOutputs;
    for (int i = 0; i < outputs.size(); i++) {
      jsonOutputs.push_back(outputs[i].toJSON());
    }
    json["outputs"] = jsonOutputs;

    return json;
  }


  void ShaderToyRenderPass::loadExternalCode(const QDir& refDir)
  {
    if (filename.isEmpty()) {
      return;
    }

    QFile file(refDir.absoluteFilePath(filename));
    if (!file.open(QIODevice::ReadOnly)) {
      return;
    }

    code = QString::fromLatin1(file.readAll());
    file.close();
  }


  void ShaderToyRenderPass::saveExternalCode(const QDir& refDir, bool overwriteExisting)
  {
    if (filename.isEmpty()) {
      return;
    }

    QFileInfo fileInfo(refDir.absoluteFilePath(filename));
    if (fileInfo.exists() && !overwriteExisting) {
      QString err = QString("cannot extract %1 to GLSL file %2, file already exists").arg(name).arg(filename);
      throw std::runtime_error(qPrintable(err));
    }


    QFile file(refDir.absoluteFilePath(filename));
    if (!file.open(QIODevice::WriteOnly)) {
      QString err = QString("unable to open GLSL file %1 for writing").arg(filename);
      throw std::runtime_error(qPrintable(err));
    }

    QByteArray contents = code.toLatin1();
    int bytesWritten = file.write(contents);
    if (bytesWritten < contents.length()) {
      QString err = QString("failed while writing %1 to GLSL file %2").arg(name).arg(filename);
      throw std::runtime_error(qPrintable(err));
    }

    file.close();
  }


  bool ShaderToyRenderPass::isValid() const
  {
    // TODO: validate other data.

    // Validate inputs
    for (int i = 0; i < inputs.size(); i++) {
      if (!inputs[i].isValid()) {
        return false;
      }
    }

    // Validate outputs
    for (int i = 0; i < outputs.size(); i++) {
      if (!outputs[i].isValid()) {
        return false;
      }
    }

    return true;
  }


  //
  // ShaderToyDocument public methods
  //

  void ShaderToyDocument::fromJSON(const QJsonObject& json)
  {
    version = json["ver"].toString();

    info.fromJSON(json["info"].toObject());

    renderpasses.clear();
    QJsonArray jsonRenderpasses = json["renderpass"].toArray();
    for (int i = 0; i < jsonRenderpasses.size(); i++) {
      ShaderToyRenderPass renderpass;
      renderpass.fromJSON(jsonRenderpasses[i].toObject());
      renderpasses.push_back(renderpass);
    }
  }


  QJsonObject ShaderToyDocument::toJSON() const
  {
    QJsonObject json;

    json["ver"]  = version;
    json["info"] = info.toJSON();

    QJsonArray jsonRenderpasses;
    for (int i = 0; i < renderpasses.size(); i++) {
      jsonRenderpasses.push_back(renderpasses[i].toJSON());
    }
    json["renderpass"] = jsonRenderpasses;

    return json;
  }


  void ShaderToyDocument::loadExternalCode()
  {
    for (int i = 0; i < renderpasses.size(); i++) {
      renderpasses[i].loadExternalCode(refDir);
    }
  }


  void ShaderToyDocument::saveExternalCode(bool overwriteExisting)
  {
    for (int i = 0; i < renderpasses.size(); i++) {
      renderpasses[i].saveExternalCode(refDir, overwriteExisting);
    }
  }


  bool ShaderToyDocument::isValid() const
  {
    if (version != QString("0.1")) {
      // We only know about version 0.1 so far...
      return false;
    }

    if (!info.isValid()) {
      return false;
    }

    // Make sure all render passes are valid.
    for (int i = 0; i < renderpasses.size(); i++) {
      if (!renderpasses[i].isValid()) {
        return false;
      }
    }

    // Must have exactly one image pass.
    if (countRenderPassesByType(kRenderPassType_Image) != 1) {
      return false;
    }

    // Can only have one common pass at most.
    if (countRenderPassesByType(kRenderPassType_Common) > 1) {
      return false;
    }

    // Can only have one cubemap pass at most.
    if (countRenderPassesByType(kRenderPassType_CubeMap) > 1) {
      return false;
    }

    // Can only have one sound pass at most.
    if (countRenderPassesByType(kRenderPassType_Sound) > 1) {
      return false;
    }

    // Can have up to 4 buffer passes.
    if (countRenderPassesByType(kRenderPassType_Buffer) > 4) {
      return false;
    }

    return true;
  }


  int ShaderToyDocument::findRenderPassByName(const QString& name, int startIndex) const
  {
    for (int i = startIndex; i < renderpasses.size(); i++) {
      if (renderpasses[i].name == name) {
        return i;
      }
    }
    return -1;
  }


  int ShaderToyDocument::findRenderPassByType(const QString& type, int startIndex) const
  {
    for (int i = startIndex; i < renderpasses.size(); i++) {
      if (renderpasses[i].type == type) {
        return i;
      }
    }
    return -1;
  }


  int ShaderToyDocument::countRenderPassesByType(const QString& type) const
  {
    int num = 0;
    for (int i = 0; i < renderpasses.size(); i++) {
      if (renderpasses[i].type == type) {
        ++num;
      }
    }
    return num;
  }


  //
  // Public functions
  //

  ShaderToyDocument* loadShaderToyJSONFile(const QString& filename)
  {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
      throw std::runtime_error("failed to open ShaderToy file for reading");
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonObject json = jsonDoc.object();
    if (json.contains("Error")) {
      QString err = QString("ShaderToy error: %1").arg(json["Error"].toString());
      throw std::runtime_error(qPrintable(err));
    }

    ShaderToyDocument* document = new ShaderToyDocument();
    QFileInfo fileInfo(filename);
    document->src = filename;
    document->refDir = fileInfo.absoluteDir();
    document->fromJSON(json["Shader"].toObject());
    if (!document->isValid()) {
      throw std::runtime_error("file contains invalid data");
    }
    document->loadExternalCode();

    return document;
  }


  void saveShaderToyJSONFile(const ShaderToyDocument* document, const QString& filename)
  {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
      throw std::runtime_error("failed to open ShaderToy file for writing");
    }

    QJsonObject json;
    json["Shader"] = document->toJSON();

    QJsonDocument jsonDoc = QJsonDocument(json);

    file.write(jsonDoc.toJson());
  }


  ShaderToyDocument* defaultShaderToyDocument()
  {
    ShaderToyDocument* document = new ShaderToyDocument();

    document->version = "0.1";

    document->info.id = "";
    document->info.date = ""; // TODO: set to the current date and time
    document->info.viewed = 0;
    document->info.name = "Untitled";
    document->info.username = ""; // TODO: save user's ShaderToy username?
    document->info.description = "";
    document->info.likes = 0;
    document->info.published = 0;
    document->info.flags = 0;
    document->info.hasliked = 0;

    ShaderToyRenderPass pass;
    pass.outputs.push_back(ShaderToyOutput{ 37, 0 });
    pass.code =
        "void mainImage( out vec4 fragColor, in vec2 fragCoord )\n"
        "{\n"
        "    // Normalized pixel coordinates (from 0 to 1)\n"
        "    vec2 uv = fragCoord/iResolution.xy;\n"
        "\n"
        "    // Time varying pixel color\n"
        "    vec3 col = 0.5 + 0.5*cos(iTime+uv.xyx+vec3(0,2,4));\n"
        "\n"
        "    // Output to screen\n"
        "    fragColor = vec4(col,1.0);\n"
        "}\n";
    pass.name = "Image";
    pass.description = "";
    pass.type = kRenderPassType_Image;
    pass.filename = "";

    document->renderpasses.push_back(pass);

    document->src = "";
    document->refDir = QDir::current();

    return document;
  }


  void extractGLSLToFiles(ShaderToyDocument* document, bool overwriteExisting)
  {
    if (document == nullptr) {
      return;
    }

    for (int i = 0; i < document->renderpasses.size(); i++) {
      ShaderToyRenderPass& pass = document->renderpasses[i];
      if (pass.code.isEmpty()) {
        continue;
      }

      if (pass.filename.isEmpty()) {
        pass.filename = QString("%1-%2.frag").arg(document->info.name).arg(pass.name).replace(' ', "");
      }

      pass.saveExternalCode(document->refDir, overwriteExisting);
    }
  }


  void inlineGLSLFromFiles(ShaderToyDocument* document)
  {
    if (document == nullptr) {
      return;
    }

    for (int i = 0; i < document->renderpasses.size(); i++) {
      ShaderToyRenderPass& pass = document->renderpasses[i];
      if (!pass.filename.isEmpty()) {
        pass.loadExternalCode(document->refDir);
        pass.filename = QString();
      }
    }
  }


  void watchAllFiles(const ShaderToyDocument* document, QFileSystemWatcher& watcher)
  {
    if (document == nullptr) {
      return;
    }

    if (!document->src.isEmpty()) {
      QFileInfo info(document->src);
      watcher.addPath(info.absoluteFilePath());
      qDebug("watching file %s", qPrintable(document->src));
    }

    for (int i = 0; i < document->renderpasses.size(); i++) {
      const ShaderToyRenderPass& pass = document->renderpasses[i];
      if (!pass.filename.isEmpty()) {
        watcher.addPath(document->refDir.absoluteFilePath(pass.filename));
        qDebug("watching file %s", qPrintable(pass.filename));
      }
    }
  }


  void roundtripJsonFile(const QString& filename, const QString& outFilename)
  {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
      throw std::runtime_error("failed to open input JSON file for roundtripping");
    }

    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());
    file.close();

    QFile outFile(outFilename);
    if (!outFile.open(QIODevice::WriteOnly)) {
      throw new std::runtime_error("failed to open output JSON file for roundtripping");
    }

    outFile.write(jsonDoc.toJson());
  }

} // namespace vh
