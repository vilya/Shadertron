#macro GLSL_VERSION

#define SHADER_TYPE_IMAGE   0
#define SHADER_TYPE_BUFFER  1
#define SHADER_TYPE_CUBEMAP 2
#define SHADER_TYPE_SOUND   3

#macro SHADER_TYPE

#macro SAMPLER_0_TYPE
#macro SAMPLER_1_TYPE
#macro SAMPLER_2_TYPE
#macro SAMPLER_3_TYPE

uniform vec3      iResolution;           // viewport resolution (in pixels)
uniform float     iTime;                 // shader playback time (in seconds)
uniform float     iTimeDelta;            // render time (in seconds)
uniform int       iFrame;                // shader playback frame
uniform float     iChannelTime[4];       // channel playback time (in seconds)
uniform vec3      iChannelResolution[4]; // channel resolution (in pixels)
uniform vec4      iMouse;                // mouse pixel coords. xy: current (if MLB down), zw: click
uniform vec4      iDate;                 // (year, month, day, time in seconds)
uniform float     iSampleRate;           // sound sample rate (i.e., 44100)

uniform SAMPLER_0_TYPE iChannel0;
uniform SAMPLER_1_TYPE iChannel1;
uniform SAMPLER_2_TYPE iChannel2;
uniform SAMPLER_3_TYPE iChannel3;

#if SHADER_TYPE == SHADER_TYPE_CUBEMAP
in vec3 fRayDir;
#endif

layout(location=0) out vec4 ShaderToolQt_oColor;

#macro COMMON_CODE

#macro USER_CODE

#if SHADER_TYPE == SHADER_TYPE_CUBEMAP

  void main()
  {
    vec2 fragCoord = gl_FragCoord.xy;
    vec3 rayOri = vec3(0.0);
    vec3 rayDir = normalize(fRayDir);
    vec4 fragColor;

    mainCubemap(fragColor, fragCoord, rayOri, rayDir);

    ShaderToolQt_oColor = fragColor;
  }

#else

  void main()
  {
    vec2 fragCoord = gl_FragCoord.xy;
    vec4 fragColor;

    mainImage(fragColor, fragCoord);

    ShaderToolQt_oColor = fragColor;
  }

#endif // (SHADER_TYPE == SHADER_TYPE_IMAGE) || (SHADER_TYPE == SHADER_TYPE_BUFFER)
