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
Assets:
- Ship with standard shadertoy assets, if their license will allow it
- Option to download standard ShaderToy assets on first run
  - Have a list of URLs, grab any that aren't already in the cache.

See 
  https://shadertoyunofficial.wordpress.com/2016/07/20/special-shadertoy-features/ 
for useful info about some of these.


Other TODO
----------

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
- Allow zooming & panning of the output image
- Save settings between runs
  - window size & position
  - Selected render size & zoom
  - Last directory used to open a file
  - ...
  

Blue sky ideas
--------------

- Support for debugging & profiling shaders.
- Save contents of an intermediate buffer as an image.
- Unlimited number of render passes, arranged in a graph.
- Allow rendering onto a triangle mesh, rather than a full screen triangle.
- Allow custom vertex shaders too.

