// Copyright 2019 Vilya Harvey
#include "TextureVideoSurface.h"

#include <QOpenGLPixelTransferOptions>

namespace vh {

  //
  // Helpers
  //

  static const char* kHandleTypeNames[] = {
    "QAbstractVideoBuffer::NoHandle",
    "QAbstractVideoBuffer::GLTextureHandle",
    "QAbstractVideoBuffer::XvShmImageHandle",
    "QAbstractVideoBuffer::CoreImageHandle",
    "QAbstractVideoBuffer::QPixmapHandle",
    "QAbstractVideoBuffer::EGLImageHandle",
  };

  QString handleTypeName(QAbstractVideoBuffer::HandleType type)
  {
    if (type >= QAbstractVideoBuffer::NoHandle && type <= QAbstractVideoBuffer::EGLImageHandle) {
      return QString::fromUtf8(kHandleTypeNames[type]);
    }
    else {
      return QString("<unknown handle type %1>").arg(int(type));
    }
  }


  static const char* kPixelFormatNames[] = {
    "Format_Invalid",
    "Format_ARGB32",
    "Format_ARGB32_Premultiplied",
    "Format_RGB32",
    "Format_RGB24",
    "Format_RGB565",
    "Format_RGB555",
    "Format_ARGB8565_Premultiplied",
    "Format_BGRA32",
    "Format_BGRA32_Premultiplied",
    "Format_BGR32",
    "Format_BGR24",
    "Format_BGR565",
    "Format_BGR555",
    "Format_BGRA5658_Premultiplied",
    "Format_AYUV444",
    "Format_AYUV444_Premultiplied",
    "Format_YUV444",
    "Format_YUV420P",
    "Format_YV12",
    "Format_UYVY",
    "Format_YUYV",
    "Format_NV12",
    "Format_NV21",
    "Format_IMC1",
    "Format_IMC2",
    "Format_IMC3",
    "Format_IMC4",
    "Format_Y8",
    "Format_Y16",
    "Format_Jpeg",
    "Format_CameraRaw",
    "Format_AdobeDng",
  };

  QString pixelFormatName(QVideoFrame::PixelFormat pixelFormat)
  {
    if (pixelFormat >= QVideoFrame::Format_Invalid && pixelFormat <= QVideoFrame::Format_AdobeDng) {
      return QString::fromUtf8(kPixelFormatNames[pixelFormat]);
    }
    else {
      return QString("<unknown pixel format %1>").arg(int(pixelFormat));
    }
  }


  QString textureFormatName(QOpenGLTexture::TextureFormat format)
  {
    switch (format) {
    case QOpenGLTexture::NoFormat:                       return QString("NoFormat");
    case QOpenGLTexture::R8_UNorm:                       return QString("R8_UNorm");
    case QOpenGLTexture::RG8_UNorm:                      return QString("RG8_UNorm");
    case QOpenGLTexture::RGB8_UNorm:                     return QString("RGB8_UNorm");
    case QOpenGLTexture::RGBA8_UNorm:                    return QString("RGBA8_UNorm");
    case QOpenGLTexture::R16_UNorm:                      return QString("R16_UNorm");
    case QOpenGLTexture::RG16_UNorm:                     return QString("RG16_UNorm");
    case QOpenGLTexture::RGB16_UNorm:                    return QString("RGB16_UNorm");
    case QOpenGLTexture::RGBA16_UNorm:                   return QString("RGBA16_UNorm");
    case QOpenGLTexture::R8_SNorm:                       return QString("R8_SNorm");
    case QOpenGLTexture::RG8_SNorm:                      return QString("RG8_SNorm");
    case QOpenGLTexture::RGB8_SNorm:                     return QString("RGB8_SNorm");
    case QOpenGLTexture::RGBA8_SNorm:                    return QString("RGBA8_SNorm");
    case QOpenGLTexture::R16_SNorm:                      return QString("R16_SNorm");
    case QOpenGLTexture::RG16_SNorm:                     return QString("RG16_SNorm");
    case QOpenGLTexture::RGB16_SNorm:                    return QString("RGB16_SNorm");
    case QOpenGLTexture::RGBA16_SNorm:                   return QString("RGBA16_SNorm");
    case QOpenGLTexture::R8U:                            return QString("R8U");
    case QOpenGLTexture::RG8U:                           return QString("RG8U");
    case QOpenGLTexture::RGB8U:                          return QString("RGB8U");
    case QOpenGLTexture::RGBA8U:                         return QString("RGBA8U");
    case QOpenGLTexture::R16U:                           return QString("R16U");
    case QOpenGLTexture::RG16U:                          return QString("RG16U");
    case QOpenGLTexture::RGB16U:                         return QString("RGB16U");
    case QOpenGLTexture::RGBA16U:                        return QString("RGBA16U");
    case QOpenGLTexture::R32U:                           return QString("R32U");
    case QOpenGLTexture::RG32U:                          return QString("RG32U");
    case QOpenGLTexture::RGB32U:                         return QString("RGB32U");
    case QOpenGLTexture::RGBA32U:                        return QString("RGBA32U");
    case QOpenGLTexture::R8I:                            return QString("R8I");
    case QOpenGLTexture::RG8I:                           return QString("RG8I");
    case QOpenGLTexture::RGB8I:                          return QString("RGB8I");
    case QOpenGLTexture::RGBA8I:                         return QString("RGBA8I");
    case QOpenGLTexture::R16I:                           return QString("R16I");
    case QOpenGLTexture::RG16I:                          return QString("RG16I");
    case QOpenGLTexture::RGB16I:                         return QString("RGB16I");
    case QOpenGLTexture::RGBA16I:                        return QString("RGBA16I");
    case QOpenGLTexture::R32I:                           return QString("R32I");
    case QOpenGLTexture::RG32I:                          return QString("RG32I");
    case QOpenGLTexture::RGB32I:                         return QString("RGB32I");
    case QOpenGLTexture::RGBA32I:                        return QString("RGBA32I");
    case QOpenGLTexture::R16F:                           return QString("R16F");
    case QOpenGLTexture::RG16F:                          return QString("RG16F");
    case QOpenGLTexture::RGB16F:                         return QString("RGB16F");
    case QOpenGLTexture::RGBA16F:                        return QString("RGBA16F");
    case QOpenGLTexture::R32F:                           return QString("R32F");
    case QOpenGLTexture::RG32F:                          return QString("RG32F");
    case QOpenGLTexture::RGB32F:                         return QString("RGB32F");
    case QOpenGLTexture::RGBA32F:                        return QString("RGBA32F");
    case QOpenGLTexture::RGB9E5:                         return QString("RGB9E5");
    case QOpenGLTexture::RG11B10F:                       return QString("RG11B10F");
    case QOpenGLTexture::RG3B2:                          return QString("RG3B2");
    case QOpenGLTexture::R5G6B5:                         return QString("R5G6B5");
    case QOpenGLTexture::RGB5A1:                         return QString("RGB5A1");
    case QOpenGLTexture::RGBA4:                          return QString("RGBA4");
    case QOpenGLTexture::RGB10A2:                        return QString("RGB10A2");
    case QOpenGLTexture::D16:                            return QString("D16");
    case QOpenGLTexture::D24:                            return QString("D24");
    case QOpenGLTexture::D24S8:                          return QString("D24S8");
    case QOpenGLTexture::D32:                            return QString("D32");
    case QOpenGLTexture::D32F:                           return QString("D32F");
    case QOpenGLTexture::D32FS8X24:                      return QString("D32FS8X24");
    case QOpenGLTexture::S8:                             return QString("S8");
    case QOpenGLTexture::RGB_DXT1:                       return QString("RGB_DXT1");
    case QOpenGLTexture::RGBA_DXT1:                      return QString("RGBA_DXT1");
    case QOpenGLTexture::RGBA_DXT3:                      return QString("RGBA_DXT3");
    case QOpenGLTexture::RGBA_DXT5:                      return QString("RGBA_DXT5");
    case QOpenGLTexture::R_ATI1N_UNorm:                  return QString("R_ATI1N_UNorm");
    case QOpenGLTexture::R_ATI1N_SNorm:                  return QString("R_ATI1N_SNorm");
    case QOpenGLTexture::RG_ATI2N_UNorm:                 return QString("RG_ATI2N_UNorm");
    case QOpenGLTexture::RG_ATI2N_SNorm:                 return QString("RG_ATI2N_SNorm");
    case QOpenGLTexture::RGB_BP_UNSIGNED_FLOAT:          return QString("RGB_BP_UNSIGNED_FLOAT");
    case QOpenGLTexture::RGB_BP_SIGNED_FLOAT:            return QString("RGB_BP_SIGNED_FLOAT");
    case QOpenGLTexture::RGB_BP_UNorm:                   return QString("RGB_BP_UNorm");
    case QOpenGLTexture::R11_EAC_UNorm:                  return QString("R11_EAC_UNorm");
    case QOpenGLTexture::R11_EAC_SNorm:                  return QString("R11_EAC_SNorm");
    case QOpenGLTexture::RG11_EAC_UNorm:                 return QString("RG11_EAC_UNorm");
    case QOpenGLTexture::RG11_EAC_SNorm:                 return QString("RG11_EAC_SNorm");
    case QOpenGLTexture::RGB8_ETC2:                      return QString("RGB8_ETC2");
    case QOpenGLTexture::SRGB8_ETC2:                     return QString("SRGB8_ETC2");
    case QOpenGLTexture::RGB8_PunchThrough_Alpha1_ETC2:  return QString("RGB8_PunchThrough_Alpha1_ETC2");
    case QOpenGLTexture::SRGB8_PunchThrough_Alpha1_ETC2: return QString("SRGB8_PunchThrough_Alpha1_ETC2");
    case QOpenGLTexture::RGBA8_ETC2_EAC:                 return QString("RGBA8_ETC2_EAC");
    case QOpenGLTexture::SRGB8_Alpha8_ETC2_EAC:          return QString("SRGB8_Alpha8_ETC2_EAC");
    case QOpenGLTexture::RGB8_ETC1:                      return QString("RGB8_ETC1");
    case QOpenGLTexture::RGBA_ASTC_4x4:                  return QString("RGBA_ASTC_4x4");
    case QOpenGLTexture::RGBA_ASTC_5x4:                  return QString("RGBA_ASTC_5x4");
    case QOpenGLTexture::RGBA_ASTC_5x5:                  return QString("RGBA_ASTC_5x5");
    case QOpenGLTexture::RGBA_ASTC_6x5:                  return QString("RGBA_ASTC_6x5");
    case QOpenGLTexture::RGBA_ASTC_6x6:                  return QString("RGBA_ASTC_6x6");
    case QOpenGLTexture::RGBA_ASTC_8x5:                  return QString("RGBA_ASTC_8x5");
    case QOpenGLTexture::RGBA_ASTC_8x6:                  return QString("RGBA_ASTC_8x6");
    case QOpenGLTexture::RGBA_ASTC_8x8:                  return QString("RGBA_ASTC_8x8");
    case QOpenGLTexture::RGBA_ASTC_10x5:                 return QString("RGBA_ASTC_10x5");
    case QOpenGLTexture::RGBA_ASTC_10x6:                 return QString("RGBA_ASTC_10x6");
    case QOpenGLTexture::RGBA_ASTC_10x8:                 return QString("RGBA_ASTC_10x8");
    case QOpenGLTexture::RGBA_ASTC_10x10:                return QString("RGBA_ASTC_10x10");
    case QOpenGLTexture::RGBA_ASTC_12x10:                return QString("RGBA_ASTC_12x10");
    case QOpenGLTexture::RGBA_ASTC_12x12:                return QString("RGBA_ASTC_12x12");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_4x4:          return QString("SRGB8_Alpha8_ASTC_4x4");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_5x4:          return QString("SRGB8_Alpha8_ASTC_5x4");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_5x5:          return QString("SRGB8_Alpha8_ASTC_5x5");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_6x5:          return QString("SRGB8_Alpha8_ASTC_6x5");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_6x6:          return QString("SRGB8_Alpha8_ASTC_6x6");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_8x5:          return QString("SRGB8_Alpha8_ASTC_8x5");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_8x6:          return QString("SRGB8_Alpha8_ASTC_8x6");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_8x8:          return QString("SRGB8_Alpha8_ASTC_8x8");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_10x5:         return QString("SRGB8_Alpha8_ASTC_10x5");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_10x6:         return QString("SRGB8_Alpha8_ASTC_10x6");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_10x8:         return QString("SRGB8_Alpha8_ASTC_10x8");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_10x10:        return QString("SRGB8_Alpha8_ASTC_10x10");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_12x10:        return QString("SRGB8_Alpha8_ASTC_12x10");
    case QOpenGLTexture::SRGB8_Alpha8_ASTC_12x12:        return QString("SRGB8_Alpha8_ASTC_12x12");
    case QOpenGLTexture::SRGB8:                          return QString("SRGB8");
    case QOpenGLTexture::SRGB8_Alpha8:                   return QString("SRGB8_Alpha8");
    case QOpenGLTexture::SRGB_DXT1:                      return QString("SRGB_DXT1");
    case QOpenGLTexture::SRGB_Alpha_DXT1:                return QString("SRGB_Alpha_DXT1");
    case QOpenGLTexture::SRGB_Alpha_DXT3:                return QString("SRGB_Alpha_DXT3");
    case QOpenGLTexture::SRGB_Alpha_DXT5:                return QString("SRGB_Alpha_DXT5");
    case QOpenGLTexture::SRGB_BP_UNorm:                  return QString("SRGB_BP_UNorm");
    case QOpenGLTexture::DepthFormat:                    return QString("DepthFormat");
    case QOpenGLTexture::AlphaFormat:                    return QString("AlphaFormat");
    case QOpenGLTexture::RGBFormat:                      return QString("RGBFormat");
    case QOpenGLTexture::RGBAFormat:                     return QString("RGBAFormat");
    case QOpenGLTexture::LuminanceFormat:                return QString("LuminanceFormat");
    case QOpenGLTexture::LuminanceAlphaFormat:           return QString("LuminanceAlphaFormat");
    default:                                             return QString("<unknown texture format %d>").arg(int(format));
    }
  }


  //
  // TextureVideoSurface public methods
  //

  TextureVideoSurface::TextureVideoSurface(QObject* parent) :
    QAbstractVideoSurface(parent)
  {
  }


  TextureVideoSurface::~TextureVideoSurface()
  {
  }


  QList<QVideoFrame::PixelFormat> TextureVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType /*type*/) const
  {
//    qDebug("supportedPixelFormats(%s)", qPrintable(handleTypeName(type)));

    QList<QVideoFrame::PixelFormat> formats;
    formats.push_back(QVideoFrame::Format_ARGB32);
    return formats;
  }


  bool TextureVideoSurface::start(const QVideoSurfaceFormat& format)
  {
//    qDebug("start(width=%d, height=%d, handleType=%s)", format.frameWidth(), format.frameHeight(), qPrintable(handleTypeName(format.handleType())));

    if (!QAbstractVideoSurface::start(format)) {
//      qDebug("start() retuned false, superclass start call failed");
      return false;
    }
    if (format.pixelFormat() != QVideoFrame::Format_ARGB32) {
//      qDebug("start() returned false, format is not ARGB32");
      return false;
    }
    _frameWidth = format.frameWidth();
    _frameHeight = format.frameHeight();
//    qDebug("start() returned true");
    _paused = false;
    return true;
  }


  void TextureVideoSurface::stop()
  {
//    qDebug("stop()");
    QAbstractVideoSurface::stop();
    _hasFrame = false;
    _paused = true;
  }


  bool TextureVideoSurface::present(const QVideoFrame& srcFrame)
  {
//    qDebug("present(width=%d, height=%d, handleType=%s, pixelFormat=%s)",
//           srcFrame.width(),
//           srcFrame.height(),
//           qPrintable(handleTypeName(srcFrame.handleType())),
//           qPrintable(pixelFormatName(srcFrame.pixelFormat()))
//    );

    if (srcFrame.width()       != _frameWidth ||
        srcFrame.height()      != _frameHeight ||
        srcFrame.pixelFormat() != QVideoFrame::Format_ARGB32) {
//      qDebug("present returned false, image size or pixel format didn't match");
      setError(IncorrectFormatError);
      return false;
    }

    if (!_paused) {
      _frame = srcFrame;
      _hasFrame = true;
    }
//    qDebug("present() returned true");
    return true;
  }


  void TextureVideoSurface::pause()
  {
    _paused = true;
  }


  void TextureVideoSurface::unpause()
  {
    _paused = false;
  }


  QVideoFrame& TextureVideoSurface::currentFrame()
  {
    return _frame;
  }


  bool TextureVideoSurface::hasCurrentFrame() const
  {
    return _hasFrame;
  }


  int TextureVideoSurface::frameWidth() const
  {
    return _frameWidth;
  }


  int TextureVideoSurface::frameHeight() const
  {
    return _frameHeight;
  }


  QSize TextureVideoSurface::frameSize() const
  {
    return QSize(_frameWidth, _frameHeight);
  }


  void TextureVideoSurface::copyToTexture(QOpenGLTexture* tex)
  {
    QOpenGLTexture::PixelFormat sourceFormat = QOpenGLTexture::RGBA;
    QOpenGLTexture::PixelType sourceType = QOpenGLTexture::UInt8;

    _frame.map(QAbstractVideoBuffer::ReadOnly);

    QOpenGLPixelTransferOptions options;
    options.setRowLength(_frame.bytesPerLine() / 4);

//    qDebug("tex: res=%dx%d, format=%s; video: res=%dx%d, %d planes, pixel format=%s",
//           tex->width(), tex->height(), qPrintable(textureFormatName(tex->format())),
//           _frame.width(), _frame.height(), _frame.planeCount(), qPrintable(pixelFormatName(_frame.pixelFormat())));
//    qDebug("video has %d bytes per line, expecting %d bytes per line",
//           _frame.bytesPerLine(), tex->width() * 4);
    tex->setData(sourceFormat, sourceType, _frame.bits(), &options);

    _frame.unmap();
  }

} // namespace vh

