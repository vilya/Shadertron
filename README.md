A program for running ShaderToy scripts on your local computer, instead of in
a web browser.

It watches the shader files in use by your project and automatically reloads
them if they change. *FIXME: this was broken during refactoring, need to bring
it back...*


Project name
------------

ShaderToolQt is a working title only, because it's pretty crap. Other ideas:
ShaderTide (a contraction of "ShaderToy IDE"), ShaderKitchen, ShaderBakery,
PixelKitchen, PixelBakery, PixelToy, FragmentToy, Pixery, Pixelhouse,
Pixelhaus, Shaderhaus, Shaderhouse.


Features
--------

- Single-pass and multi-pass image shaders
- Common source for all passes
- Input sources:
  - Texture
  - Cube map
  - Mouse
  - Keyboard
- Can toggle keyboard & mouse input to shaders on or off (off is useful when you want to use keyboard shortcuts, for example).
- Uses ShaderToy's JSON format (as returned by their API) as the native file format, with some minor extensions.
  - Supports external file references
  - Can automatically extract GLSL into external files
  - Can automatically inline GLSL from external files back into the JSON.
- Playback controls:
  - Play, pause, restart
  - Fast forward/rewind in small, medium or large increments
- Overlay showing playback and performance stats
- User-specified rendering resolution
- Zoom


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


See 
  https://shadertoyunofficial.wordpress.com/2016/07/20/special-shadertoy-features/ 
for useful info about some of these.


A note about driver-level incompatibilities
-------------------------------------------

There are some driver-level incompatibilities which can trip us up. I've hit
two examples so far:

- KifsOctahedron had arguments to `clamp` in the wrong order, leading to
  undefined behaviour. Intel & WebGL shader compilers accept this, the Nvidia
  shader compiler gives incorrect output.

- WordToy used `char` as an identifier, but it's actually a reserved word in 
  GLSL. Nvidia shader compiler treats this as a compile error, the WebGL 
  compiler accepts it.

It's not really practical to catch all of these cases. Maybe using GLES
instead would give a more compatible result? It's kind of nice to have GL 4.5
though...


Other TODO
----------

- Overlay to display shader metadata 
- Log window to display messages about shaders which didn't compile, etc.
- Validate the JSON as we load.
- Overlay to display per-shader inputs and outputs.
- Save screenshots.
- Save high quality renders.
- Save video.
- Option to run the shaders for N frames before saving an image/video
- HDR texture inputs
- Display at pre-defined resolutions (not necessarily the full window)
- Full-screen mode
- VR support
  - Image shaders render on a floating plane in space
  - VR shaders are fully immersive
- Download directly from ShaderToy.
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


ShaderToy JSON Notes
====================

Renderpasses
------------
- The list of renderpasses is sparse and (maybe?) isn't guaranteed to be in order. 
- Use the `name` field to identify renderpasses
- The final renderpass always has `name:"Image"`.
- Buffer A can be called "Buf A" or "Buffer A" (I think). Need to check for both.
- Renderpass types can be:
  - "image"
  - "buffer"
  - ...others for audio, vr, cubemaps, etc?
- Common source code is listed as a renderpass with name = "Common" and type = "common".
  - This renderpass has no inputs or outputs.
  - This is the *only* renderpass that does not have any outputs.

Inputs
------
- The list of inputs is sparse and not guaranteed to be in order.
- The `channel` field identifies which input it is.
- The `id` field is used to identify the input source.
  - `id > 256`  seems to be a reference to a render pass.
  - Render pass outputs have an `id` too. Presumably we can match the input `id` up with this.
  - The provided assets all seem to have a fixed ID across all shaders.
- The `ctype` field identifies what type of input we're dealing with:
  - "buffer" for a reference to one of the renderpass outputs
  - "cubemap" for a static cubemap texture
  - "texture" for a static 2D texture
  - presumably others for audio, 3D textures, etc.

Outputs
-------
- It looks like maybe there was a plan to allow for multiple outputs, but it has not been implemented?
  - Or perhaps it's used for shader types that I haven't encountered yet?
- The list of outputs always has length 1, except for the common code pass where it has length 0.
- Outputs have an `id` and a channel.
- So far the channel is always 0.
- The output `id` is (so far) one of the following values:
  - 37 for the "image" renderpass
  - 257 for Buf A
  - 258 for Buf B
  - Presumably 259 and 260 for Buf C and Buf D respectively.
