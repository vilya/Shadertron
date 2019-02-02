// Copyright 2019 Vilya Harvey
#ifndef VH_TEXTUREAUDIOSURFACE_H
#define VH_TEXTUREAUDIOSURFACE_H

#include <QAudioBuffer>
#include <QObject>
#include <QOpenGLTexture>

namespace vh {

  class TextureAudioSurface : public QObject
  {
    Q_OBJECT
  public:
    explicit TextureAudioSurface(QObject *parent = nullptr);
    virtual ~TextureAudioSurface();

    QAudioBuffer& currentBuffer();
    bool hasCurrentBuffer() const;

    void pause();
    void unpause();

    void copyToTexture(QOpenGLTexture* tex, qint64 playbackTime);

  public slots:
    void audioBufferReady(const QAudioBuffer& buffer);
    void audioFlushed();

  private:
    QAudioBuffer _buffer;
    bool _hasBuffer = false;
    bool _paused    = false;
    short _texData[2][512];
  };

} // namespace vh

#endif // VH_TEXTUREAUDIOSURFACE_H
