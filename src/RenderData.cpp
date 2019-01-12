// Copyright 2019 Vilya Harvey
#include "RenderData.h"

namespace vh {

  //
  // Texture public methods
  //

  QString Texture::samplerType(int samplerNum) const
  {
    QString val = QString::fromUtf8("#define SAMPLER_%1_TYPE %2");

    switch (obj->target()) {
    case QOpenGLTexture::Target2D:
      val = val.arg(samplerNum).arg("sampler2D");
      break;
    case QOpenGLTexture::Target1D:
      val = val.arg(samplerNum).arg("sampler1D");
      break;
    case QOpenGLTexture::Target3D:
      val = val.arg(samplerNum).arg("sampler3D");
      break;
    case QOpenGLTexture::TargetCubeMap:
      val = val.arg(samplerNum).arg("samplerCube");
      break;
    default:
      val = val.arg(samplerNum).arg("sampler2D");
      break;
    }

    return val;
  }

} // namespace vh
