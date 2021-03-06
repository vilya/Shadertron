// Copyright 2019 Vilya Harvey
#ifndef VH_RENDERDATA_H
#define VH_RENDERDATA_H

#include <QAudioProbe>
#include <QCamera>
#include <QMediaPlayer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

#include "TextureAudioSurface.h"
#include "TextureVideoSurface.h"

namespace vh {

  //
  // Constants
  //

  static constexpr int kTexture_PlaceholderImage    = 0;
  static constexpr int kTexture_PlaceholderCubemap  = 1;
  static constexpr int kTexture_Keyboard            = 2;

  static constexpr int kNumSpecialTextures = kTexture_Keyboard + 1;

  static constexpr int kMaxInputs       = 4;
  static constexpr int kMaxRenderpasses = 5;
  static constexpr int kMaxVideos       = 4;
  static constexpr int kMaxAudios       = 4;
  static constexpr int kMaxCameras      = 1;
  static constexpr int kMaxTextures     = kMaxRenderpasses * (2 + kMaxInputs) + (kMaxVideos * 2) + kNumSpecialTextures;

  static constexpr int kCubemapWidth  = 1024;
  static constexpr int kCubemapHeight = 1024;


  //
  // Enums
  //

  enum class PassType {
    eImage,
    eBuffer,
    eCubemap,
    eSound,
  };


  //
  // Structs
  //

  struct TextureReference {
    int id;
    bool srgb;
    bool flip;

    bool operator == (const TextureReference& other) const { return id == other.id && srgb == other.srgb && flip == other.flip; }
    bool operator != (const TextureReference& other) const { return id != other.id || srgb != other.srgb || flip != other.flip; }
  };


  inline uint qHash(const TextureReference& tr)
  {
    uint h = ::qHash(qMakePair(tr.id, tr.srgb));
    return ::qHash(qMakePair(h, tr.flip));
  }


  struct Texture {
    QOpenGLTexture* obj = nullptr;
    bool isRenderSized  = false;  // if true, this will be resized dynamically to match our render resolution.
    float playbackTime  = 0.0f;   // for animated channels, this is the current time on the channel.

    QString samplerType(int samplerNum) const;
  };


  struct Video {
    QMediaPlayer* player         = nullptr;
    TextureVideoSurface* surface = nullptr;
    int texOutput                = -1;      // Index of the texture that the un-flipped video will be written into.
    int flippedTexOutput         = -1;      // Index of the texture that the flipped video will be written into.
  };


  struct Camera {
    QCamera* obj                 = nullptr;
    TextureVideoSurface* surface = nullptr;
    int texOutput                = -1;      // Index of the texture that the un-flipped video will be written into.
    int flippedTexOutput         = -1;      // Index of the texture that the flipped video will be written into.
  };


  struct Audio {
    QMediaPlayer* player         = nullptr;
    QAudioProbe* probe           = nullptr;
    TextureAudioSurface* surface = nullptr;
    int texOutput                = -1;
  };



  struct RenderPass {
    PassType type;
    QString name;
    int outputID; // The ID for the output of this render pass, taken from the ShaderToy file.

    QOpenGLShaderProgram* program = nullptr;

    int inputs[kMaxInputs][2]     = {}; // Front and back textures for each input.
    int outputs[2]                = {}; // Front and back output textures.

    GLuint samplers[kMaxInputs] = {}; // Samplers used for each input.

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

    int iRayDirsLoc           = -1;
  };


  struct TexturedQuadShader {
    QOpenGLShaderProgram* program = nullptr;

    // Uniform locations
    int iResolutionLoc = -1;
    int iShapeLoc = -1;
  };


  struct RenderData {
    Video videos[kMaxVideos]                  = {};
    Audio audios[kMaxAudios]                  = {};
    Texture textures[kMaxTextures]            = {}; // Element 0 will be a special "no texture" value.
    RenderPass renderpasses[kMaxRenderpasses] = {}; // In order: all of the intermediate buffers followed by the final output.
    int numVideos       = 0;
    int numAudios       = 0;
    int numTextures     = 0;
    int numRenderpasses = 0;

    Camera camera   = {};
    bool hasCamera  = false;

    GLuint defaultVAO   = 0;
    GLuint defaultFBO   = 0;
    GLuint flipFBO      = 0;
    GLuint grabFBO      = 0;

    uint8_t backBuffer  = 0;
    uint8_t frontBuffer = 1;

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

    // Source data for keyboard texture.
    uchar keyboardTexData[3][256];

    // Utility shaders
    TexturedQuadShader texturedQuadShader;
  };

} // namespace vh

#endif // VH_RENDERDATA_H
