// Copyright 2019 Vilya Harvey
#include "ShaderToy.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
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
    // TODO
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
    // TODO
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
    // TODO
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

    QFileInfo fileInfo(filename);
    if (!fileInfo.isAbsolute()) {
      filename = refDir.absoluteFilePath(filename);
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
      return;
    }

    code = QString::fromLatin1(file.readAll());
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


  bool ShaderToyDocument::isValid() const
  {
    // TODO: validate the version field.

    if (!info.isValid()) {
      return false;
    }

    // Validate render passes
    for (int i = 0; i < renderpasses.size(); i++) {
      if (!renderpasses[i].isValid()) {
        return false;
      }
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

    ShaderToyDocument* document = new ShaderToyDocument();
    QFileInfo fileInfo(filename);
    document->src = filename;
    document->refDir = fileInfo.absoluteDir();
    document->fromJSON(json["Shader"].toObject());

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
