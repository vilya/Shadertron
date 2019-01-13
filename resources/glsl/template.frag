#version 450 core

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

layout(location=0) out vec4 ShaderToolQt_oColor;

#macro COMMON_CODE

#macro USER_CODE

#macro MAIN
