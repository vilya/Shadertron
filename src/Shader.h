// Copyright 2019 Vilya Harvey
#ifndef VH_SHADER_H
#define VH_SHADER_H

#include <QDir>
#include <QFileSystemWatcher>
#include <QImage>
#include <QJsonObject>
#include <QObject>
#include <QString>

#include <stdexcept>

namespace vh {

  static constexpr int kMaxPasses = 5;
  static constexpr int kMaxInputs = 4;


  struct ShaderInfo {
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
    bool hasLiked;

    void loadJSON(const QJsonObject& json);
  };


  struct Sampler {
    QString filter;
    QString wrap;
    bool vflip;
    bool srgb;
    QString internal;

    void loadJSON(const QJsonObject& json);
  };


  enum class RenderPassInputType {
    eNone,
    eTexture,
    eRenderPass
  };


  struct RenderPassInput {
    int id;
    QString src;
    RenderPassInputType type = RenderPassInputType::eNone;
    int channel;
    Sampler sampler;
    bool published;

    void loadJSON(const QJsonObject& json);
  };


  struct RenderPassOutput {
    int id;
    int channel;

    void loadJSON(const QJsonObject& json);
  };


  struct RenderPass {
    RenderPassInput inputs[kMaxInputs];
    RenderPassOutput output;
    QString code;
    QString name;
    QString description;

    // Indicates the type of shader to expect in the render pass. Values
    // encountered so far: "image", "buffer" and "common".
    QString type;

    // Additional field, which lets you specify an external file to read the
    // shader source code from (and watch for changes), instead of having the
    // code embedded directly in the JSON file.
    //
    // You should specify *either* this *or* the `code` field in your JSON. If
    // you provide both, the `code` field will be ignored and we will use the
    // contents of the external file instead.
    QString filename;

    void loadJSON(const QJsonObject& json, const QDir& refDir);
  };


  /// In ShaderToy terminology, a `Shader` is a collection of render passes and
  /// assets which produces a final image for display. It's *not* the same as
  /// an OpenGL shader. Each *render pass* has a single OpenGL shader, so the
  /// `Shader` object is actually a collection of OpenGL shaders which get
  /// called in a pre-defined order.
  class Shader : public QObject
  {
    Q_OBJECT
  public:
    static constexpr int kPass_Final   = 0;
    static constexpr int kPass_A = 1;
    static constexpr int kPass_B = 2;
    static constexpr int kPass_C = 3;
    static constexpr int kPass_D = 4;

  public:
    explicit Shader(QObject *parent = nullptr);
    explicit Shader(const QString& jsonFilename, QObject *parent = nullptr);

    bool hasPass(int passIdx) const;
    QString passSource(int passIdx) const;

    bool hasPassInput(int passIdx, int inputIdx) const;
    RenderPassInputType inputType(int passIdx, int inputIdx) const;
    QImage inputImage(int passIdx, int inputIdx) const;

  signals:
    void passSourceChanged(int passIdx);
    void commonSourceChanged();

    void inputImageChanged(int passIdx, int inputIdx);

  private slots:
    void handleSourceFileChange(const QString& filename);

  private:
    void loadJSON(const QJsonObject& json, const QDir& refDir);

  private:
    QString _ver;
    ShaderInfo _shaderInfo;
    RenderPass _renderpasses[kMaxPasses];

    QString _sourceCode[kMaxPasses];
    QString _commonSourceCode;

    QString _sourceFiles[kMaxPasses];
    QString _commonSourceFile;

    QFileSystemWatcher _fileWatcher;
  };

} // namespace vh

#endif // VH_SHADER_H
