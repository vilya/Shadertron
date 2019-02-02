// Copyright 2019 Vilya Harvey
#include "TextureAudioSurface.h"

#include <QAudioFormat>
#include <QOpenGLPixelTransferOptions>

namespace vh {

  //
  // TextureAudioSurface public methods
  //

  TextureAudioSurface::TextureAudioSurface(QObject* parent) :
    QObject(parent)
  {
  }


  TextureAudioSurface::~TextureAudioSurface()
  {
  }


  void TextureAudioSurface::pause()
  {
    _paused = true;
  }


  void TextureAudioSurface::unpause()
  {
    _paused = false;
  }


  QAudioBuffer& TextureAudioSurface::currentBuffer()
  {
    return _buffer;
  }


  bool TextureAudioSurface::hasCurrentBuffer() const
  {
    return _hasBuffer;
  }


  void TextureAudioSurface::copyToTexture(QOpenGLTexture* tex, qint64 playbackTime)
  {
//    qDebug("Copy audio buffer to texture. Buffer has: "
//           "%d channels, %d frames, %.3lf KHz sample rate, starts at %.3lf ms and plays for %.3lf ms",
//           _buffer.format().channelCount(),
//           _buffer.frameCount(),
//           double(_buffer.format().sampleRate()) / 1000.0,
//           double(_buffer.startTime()) / 1000.0,
//           double(_buffer.duration()) / 1000.0
//    );

    qint64 startOffsetMicrosecs = playbackTime - _buffer.startTime();
    if (startOffsetMicrosecs < 0) {
      startOffsetMicrosecs = 0;
    }

    // Sample rate is in Hz, i.e. samples per second. Our start offset is
    // in microseconds so we need to divide it by 1,000,000 to convert it
    // into seconds. Doing the division last to preserve accuracy.
    qint64 startFrame = _buffer.format().sampleRate() * startOffsetMicrosecs / 1000000;
    qint64 endFrame = startFrame + 512;
    if (endFrame > _buffer.frameCount()) {
      endFrame = _buffer.frameCount();
      startFrame = endFrame - 512;
      if (startFrame < 0) {
        startFrame = 0;
      }
    }

//    qDebug("audio start frame = %lld, end frame = %lld", startFrame, endFrame);

    const short* sourceData = reinterpret_cast<const short*>(_buffer.constData());
    const int stride = _buffer.format().channelCount();
    int i = 0, j = 0;
    for (qint64 frame = startFrame; frame < endFrame; frame++) {
      _texData[1][i] = sourceData[j] / 2 + (1 << 14);
      ++i;
      j += stride;
    }

    // TODO: This is just here as a placeholder. _texData[0] is supposed to hold the FFT of the waveform.
    memcpy(_texData[0], _texData[1], 512 * sizeof(short));

    QOpenGLTexture::PixelFormat sourceFormat = QOpenGLTexture::Red;
    QOpenGLTexture::PixelType sourceType = QOpenGLTexture::Int16;

    QOpenGLPixelTransferOptions options;

    tex->setData(sourceFormat, sourceType, _texData, &options);
  }


  //
  // TextureAudioSurface public slots
  //

  static const char* sampleTypeNames[] = {
    "unknown",
    "signed int",
    "unsigned int",
    "float"
  };

  void TextureAudioSurface::audioBufferReady(const QAudioBuffer& buffer)
  {
    if (!_paused) {
      _buffer = buffer;
      if (!_hasBuffer) {
        int st = int(_buffer.format().sampleType());
        const char* sampleTypeName = (st >= 0 && st <= 3) ? sampleTypeNames[st] : "<invalid type>";

        qDebug("First audio buffer received.");
        qDebug("Audio buffer has: %d channels, %d frames, %.3lf KHz sample rate, starts at %.3lf ms and plays for %.3lf ms",
               _buffer.format().channelCount(),
               _buffer.frameCount(),
               double(_buffer.format().sampleRate()) / 1000.0,
               double(_buffer.startTime()) / 1000.0,
               double(_buffer.duration()) / 1000.0);
        qDebug("Audio buffer is %s", (_buffer.format().byteOrder() == QAudioFormat::LittleEndian) ? "little-endian" : "big-endian");
        qDebug("Audio buffer samples are %d bit %s", _buffer.format().sampleSize(), sampleTypeName);
      }
      _hasBuffer = true;
    }
  }


  void TextureAudioSurface::audioFlushed()
  {
    // TODO - is anything necessary here?
  }

} // namespace vh
