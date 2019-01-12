// Copyright 2019 Vilya Harvey
#ifndef VH_RENDERDATA_H
#define VH_RENDERDATA_H

#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

namespace vh {

  //
  // Constants
  //

  static constexpr int kMaxInputs       = 4;
  static constexpr int kMaxRenderpasses = 5;
  static constexpr int kMaxTextures     = kMaxRenderpasses * (2 + kMaxInputs) + 1;


  //
  // Structs
  //

  struct Texture {
    QOpenGLTexture* obj = nullptr;
    bool isBuffer       = false;  // if true, this will be resized dynamically to match our output resolution.
    float playbackTime  = 0.0f;   // for animated channels, this is the current time on the channel.

    QString samplerType(int samplerNum) const;
  };


  struct RenderPass {
    QOpenGLShaderProgram* program = nullptr;
    int inputs[kMaxInputs][2]     = {}; // Front and back textures for each input.
    int outputs[2]                = {}; // Front and back output textures.

    QString sourceCode;
    QString sourceFile;

    // Uniform indexes
    int iResolutionLoc        = -1;
    int iTimeLoc              = -1;
    int iTimeDeltaLoc         = -1;
    int iFrameLoc             = -1;
    int iMouseLoc             = -1;
    int iChannelTimeLoc       = -1;
    int iChannelResolutionLoc = -1;
    int iChannel0Loc          = -1;
    int iChannel1Loc          = -1;
    int iChannel2Loc          = -1;
    int iChannel3Loc          = -1;
    int iDateLoc              = -1;
    int iSampleRateLoc        = -1;
  };


  struct RenderData {
    Texture textures[kMaxTextures]            = {}; // Element 0 will be a special "no texture" value.
    RenderPass renderpasses[kMaxRenderpasses] = {}; // In order: all of the intermediate buffers followed by the final output.
    int numTextures     = 0;
    int numRenderpasses = 0;

    GLuint defaultVAO    = 0;
    GLuint defaultFBO    = 0;

    uint8_t backBuffer   = 0;
    uint8_t frontBuffer  = 1;

    QString commonSourceCode;
    QString commonSourceFile;

    // Global uniform values.
    float iResolution[3] = { 0.0f, 0.0f, 0.0f };
    float iTime          = 0.0f;
    float iTimeDelta     = 0.0f;
    int   iFrame         = 0.0f;
    float iMouse[4]      = { 0.0f, 0.0f, -1.0, -1.0f };
    float iDate[4]       = { 0.0f, 0.0f, 0.0f, 0.0f };
    float iSampleRate    = 0.0f;
  };

} // namespace vh

#endif // VH_RENDERDATA_H
