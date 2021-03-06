// Copyright 2019 Vilya Harvey
#ifndef VH_SHADERTOY_H
#define VH_SHADERTOY_H

// Code for loading and saving ShaderToy's JSON documents.

#include <QDir>
#include <QFileSystemWatcher>
#include <QString>
#include <QStringList>
#include <QVector>

// Forward declarations of Qt classes
class QJsonObject;


namespace vh {

  //
  // Constants
  //

  static const QString kShaderToyAppKey = "fdHtWW";

  static const QString kRenderPassType_Common  = "common";
  static const QString kRenderPassType_Buffer  = "buffer";
  static const QString kRenderPassType_Image   = "image";
  static const QString kRenderPassType_CubeMap = "cubemap";
  static const QString kRenderPassType_Sound   = "sound";

  static const QString kInputType_Buffer    = "buffer";
  static const QString kInputType_CubeMap   = "cubemap";
  static const QString kInputType_Keyboard  = "keyboard";
  static const QString kInputType_Texture   = "texture";
  static const QString kInputType_Video     = "video";
  static const QString kInputType_Webcam    = "webcam";
  static const QString kInputType_Music     = "music";

  static const QString kSamplerFilterType_Nearest = "nearest";
  static const QString kSamplerFilterType_Linear  = "linear";
  static const QString kSamplerFilterType_Mipmap  = "mipmap";

  static const QString kSamplerWrapType_Clamp = "clamp";
  static const QString kSamplerWrapType_Repeat = "repeat";


  // IDs of the output associated with each of the standard passes. The common
  // pass doesn't have any outputs, so has no output ID.
  static const int kOutputID_Image = 37;
  static const int kOutputID_Sound = 38;
  static const int kOutputID_CubeA = 41;
  static const int kOutputID_BufA = 257;
  static const int kOutputID_BufB = 258;
  static const int kOutputID_BufC = 259;
  static const int kOutputID_BufD = 260;


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
    void saveExternalCode(const QDir& refDir, bool overwriteExisting);

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
    void saveExternalCode(bool overwriteExisting);

    bool isValid() const;

    int findRenderPassByOutputID(int outputID) const;
    int findRenderPassByName(const QString& name, int startIndex=0) const;
    int findRenderPassByType(const QString& type, int startIndex=0) const;

    int countRenderPassesByType(const QString& type) const;
  };


  ShaderToyDocument* loadShaderToyJSONFile(const QString& filename);
  void saveShaderToyJSONFile(const ShaderToyDocument* document, const QString& filename);

  ShaderToyDocument* defaultShaderToyDocument();

  void extractGLSLToFiles(ShaderToyDocument* document, bool overwriteExisting);
  void inlineGLSLFromFiles(ShaderToyDocument* document);
  void watchAllFiles(const ShaderToyDocument* document, QFileSystemWatcher& watcher);

  void roundtripJsonFile(const QString& filename, const QString& outFilename);

} // namespace vh

#endif // VH_SHADERTOY_H
