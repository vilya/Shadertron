ShaderToy Compatibility TODO
----------------------------

Shader types:
- Audio
- Cubemap
- VR
Input sources:
- 3D texture
- Video stream/file
- Audio stream/file
- Webcam input
- Microphone input
- Soundcloud input
Sampler properties
- flip (need to handle the same texture with different flip settings)
- srgb (ditto)
Assets:
- Ship with standard shadertoy assets, if their license will allow it
- Option to download standard ShaderToy assets on first run
  - Have a list of URLs, grab any that aren't already in the cache.

See 
  https://shadertoyunofficial.wordpress.com/2016/07/20/special-shadertoy-features/ 
for useful info about some of these.


Other TODO
----------

- Shader editing
  - Add/remove passes
  - Select inputs
  - Open shader code in external text editor (user configurable)
  - Modify metadata
  - Project templates (i.e. create a new shader with N passes & specified code in each pass)
  	- e.g. a template which raymarches an SDF, where you only have to fill in the scene modelling functions
  - GLSL snippets
  	- e.g. library of distance functions, raymarching code, analytic intersection tests, etc.
  	- user provided snippets
  	- online snippet sources - google "online code snippet manager" and see what comes up
- When intermediate outputs are showing, click on one to make that the current display pass.
- Overlay to display shader metadata 
- Log window to display messages about shaders which didn't compile, etc.
- Validate the JSON as we load.
- Overlay to display render pass inputs.
- Save screenshots.
- Save high quality renders.
- Save video.
- Option to run the shaders for N frames before saving an image/video.
- HDR texture inputs.
- Full-screen mode
- VR support
  - Image shaders render on a floating plane in space
  - VR shaders are fully immersive
- Upload to ShaderToy (if their API allows it?)
- Make it easy to import custom assets
  - Asset library?
- Save settings between runs
  - Last directory used to open a file
  - ...


Blue sky ideas
--------------

- Support for debugging & profiling shaders.
- Save contents of an intermediate buffer as an image.
- Unlimited number of render passes, arranged in a graph.
- Allow rendering onto a triangle mesh, rather than a full screen triangle.
- Custom vertex shaders
- Buffers as input (i.e. SSBOs and/or tex buffers)
- Compute shaders generating an arbitrary output buffer.
