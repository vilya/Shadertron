// Copyright 2019 Vilya Harvey
#ifndef VH_SHADERTOY_H
#define VH_SHADERTOY_H

// Code for loading and saving ShaderToy's JSON documents.

#include <QDir>
#include <QString>
#include <QStringList>
#include <QVector>

// Forward declarations of Qt classes
class QJsonObject;


namespace vh {

  //
  // Constants
  //

  static const QString kRenderPassType_Common  = "common";
  static const QString kRenderPassType_Buffer  = "buffer";
  static const QString kRenderPassType_Image   = "image";
  static const QString kRenderPassType_CubeMap = "cubemap";
  static const QString kRenderPassType_Sound   = "sound";

  static const QString kInputType_Buffer  = "buffer";
  static const QString kInputType_CubeMap = "cubemap";
  static const QString kInputType_Texture = "texture";


  //
  // Structs
  //

  struct ShaderToySampler {
    QString filter;
    QString wrap;
    QString vflip;
    QString srgb;
    QString internal;

    void fromJSON(const QJsonObject& json);
    QJsonObject toJSON() const;

    bool isValid() const;
  };


  struct ShaderToyInput {
    int id;
    QString src;
    QString ctype;
    int channel;
    ShaderToySampler sampler;
    int published;

    void fromJSON(const QJsonObject& json);
    QJsonObject toJSON() const;

    bool isValid() const;
  };


  struct ShaderToyOutput {
    int id;
    int channel;

    void fromJSON(const QJsonObject& json);
    QJsonObject toJSON() const;

    bool isValid() const;
  };


  struct ShaderToyInfo {
    QString id;
    QString date;
    int viewed;
    QString name;
    QString username;
    QString description;
    int likes;
    int published;
    int flags;
    QStringList tags;
    int hasliked;

    void fromJSON(const QJsonObject& json);
    QJsonObject toJSON() const;

    bool isValid() const;
  };


  struct ShaderToyRenderPass {
    QVector<ShaderToyInput> inputs;
    QVector<ShaderToyOutput> outputs;
    QString code;
    QString name;
    QString description;
    QString type;

    QString filename; // optional, non-standard.

    void fromJSON(const QJsonObject& json);
    QJsonObject toJSON() const;

    void loadExternalCode(const QDir& refDir);

    bool isValid() const;
  };


  struct ShaderToyDocument {
    QString version;
    ShaderToyInfo info;
    QVector<ShaderToyRenderPass> renderpasses;

    // Runtime-only data, not stored in JSON.
    QString src; // The location the document was read from, i.e. a filename or URL.
    QDir refDir; // The directory to use for resolving any relative filenames in the document.

    void fromJSON(const QJsonObject& json);
    QJsonObject toJSON() const;

    void loadExternalCode();

    bool isValid() const;

    int findRenderPassByName(const QString& name, int startIndex=0) const;
    int findRenderPassByType(const QString& type, int startIndex=0) const;
  };


  ShaderToyDocument* loadShaderToyJSONFile(const QString& filename);
  void saveShaderToyJSONFile(const ShaderToyDocument* document, const QString& filename);

  void roundtripJsonFile(const QString& filename, const QString& outFilename);

} // namespace vh

#endif // VH_SHADERTOY_H
