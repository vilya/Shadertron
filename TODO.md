ShaderToy Compatibility TODO
----------------------------

Shader types:
- Audio
- Cubemap
- VR
Input sources:
- 3D texture
- Audio stream/file
- Microphone input
- Soundcloud input
Assets:
- Option to download standard ShaderToy assets on first run
- Have a list of URLs, grab any that aren't already in the cache.
  ... if this is allowed by their license?


See 
  https://shadertoyunofficial.wordpress.com/2016/07/20/special-shadertoy-features/ 
for useful info about some of these.


TODO: video input support
-------------------------
- Widgets to control playback for each video stream
- Option to unmute/mute each video.
- Add support for more pixel formats instead of forcing the video to ARGB32.


TODO: downloading from ShaderToy
--------------------------------
- Add a search form
  - Support built in sort types: name, love, popular, newest & hot
  - Support built in filter types: vr, soundoutput, soundinput, webcam, multipass, musicstream
  - Use paging to show results.
- Download and cache thumbnails for shaders:
  - https://www.shadertoy.com/media/shaders/<shader-id>.jpg
- Figure out how to tell when a cached file is out of date.
- Show download progress


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
  - ...


Architecture TODO
-----------------

- Allow for several OpenGL widget subclasses all accessing the same render data.


Blue sky ideas
--------------

- Support for debugging & profiling shaders.
- Save contents of an intermediate buffer as an image.
- Unlimited number of render passes, arranged in a graph.
- Allow rendering onto a triangle mesh, rather than a full screen triangle.
- Custom vertex shaders
- Buffers as input (i.e. SSBOs and/or tex buffers)
- Compute shaders generating an arbitrary output buffer.

- For each render pass, specify whether its graphics, compute or raytracing
- For graphics, allow user to specify the complete pipeline:
  - input geometry (procedural or user-supplied meshes) 
  - custom shaders for each stage (vertex, geometry, tesselation, fragment)
- Similarly for raytracing
