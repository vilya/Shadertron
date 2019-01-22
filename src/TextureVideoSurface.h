// Copyright 2019 Vilya Harvey
#ifndef VH_TEXTUREVIDEOSURFACE_H
#define VH_TEXTUREVIDEOSURFACE_H

#include <QAbstractVideoBuffer>
#include <QAbstractVideoSurface>
#include <QList>
#include <QOpenGLTexture>
#include <QVideoSurfaceFormat>

namespace vh {

  class TextureVideoSurface : public QAbstractVideoSurface
  {
  public:
    TextureVideoSurface(QObject* parent=nullptr);
    virtual ~TextureVideoSurface();

    virtual QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const;

    virtual bool start(const QVideoSurfaceFormat &format);
    virtual void stop();

    virtual bool present(const QVideoFrame &frame);

    QVideoFrame& currentFrame();
    bool hasCurrentFrame() const;
    int frameWidth() const;
    int frameHeight() const;
    QSize frameSize() const;

    void copyToTexture(QOpenGLTexture* tex);

  private:
    QVideoFrame _frame;
    bool _hasFrame   = false;
    int _frameWidth  = 0;
    int _frameHeight = 0;
  };

} // namespace vh

#endif // VH_TEXTUREVIDEOSURFACE_H
