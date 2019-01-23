Run ShaderToy scripts on your local computer, instead of in a web browser. 

The goal is to be 100% compatible with the complete ShaderToy feature set.
This is still in progress; see TODO.md for what is & isn't supported so far.

We also provide some useful extensions to ShaderToy's capabilities: 
* GLSL code can be stored in external files
* You can edit the code in a text editor of your choice. Wehn you save the
  file, we'll detect the change and aautomaatically reload.
* You can provide your own assets rather than having to use ShaderToy's built
  in ones (although you can use the built-in ones too).


Project name
------------

Still undecided. ShaderToolQt is a working title only, it's pretty crap. I do
still like plain old ShaderTool though.

Other ideas: ShaderTide, ShaderKitchen, ShaderBakery, PixelKitchen,
PixelBakery, PixelToy, FragmentToy, Pixery, Pixelhouse, Pixelhaus, Shaderhaus,
Shaderhouse, Nightshade, N-Shade, V-Shade,


Features
--------

- Single-pass and multi-pass image shaders
- Common source for all passes
- Input sources:
  - Texture
  - Video
  - Cube map
  - Mouse
  - Keyboard
- Support for all of ShaderToy's sampler properties:
  - Filter (mipmap, linear or nearest)
  - Wrap (repeat or clamp)
  - sRGB
  - Flip
- Can toggle keyboard & mouse input to shaders on or off (off is useful when you want to use keyboard shortcuts, for example).
- Uses ShaderToy's JSON format (as returned by their API) as the native file format, with some minor extensions.
  - Supports external file references
  - Can automatically extract GLSL into external files
  - Can automatically inline GLSL from external files back into the JSON.
- Download directly from ShaderToy
   - Downloads referenced assets too
   - Can cache all standard assets ahead of time (in case you're going offline)
- Playback controls:
  - Play, pause, restart
  - Fast forward/rewind in small, medium or large increments
- Overlays showing additional info:
  - HUD showing a customisable set of playback and performance stats
  - Outputs from all intermediate render passes
  - Inputs to the render pass currently being displayed.
- User-specified rendering resolution
- Pan and Zoom the output


Video support
-------------

Some of ShaderToy's videos are in .ogv format & others are in .webm. On
Windows you may need to install additional codecs to be able to play back
these formats.

For more information about codecs, including how to find out which ones are
available on your system, see:

https://support.microsoft.com/en-gb/help/15070/windows-media-player-codecs-frequently-asked-questions

The K-Lite Codec Pack includes support for .ogv and .webm as well as a variety
of other video formats. It's regularly updated and seems trustworthy. It's
available from here:

http://www.codecguide.com/about_kl.htm


A note about GLSL vs. ESSL incompatibilities
--------------------------------------------

There are some language- and driver-level incompatibilities between WebGL and
native GL running locally which can trip us up. I've hit these examples so
far:

- KifsOctahedron had arguments to `clamp` in the wrong order, leading to
  undefined behaviour. Intel & WebGL shader compilers accept this, the Nvidia
  shader compiler gives incorrect output.

- WordToy used `char` as an identifier, but it's actually a reserved word in 
  GLSL. Nvidia shader compiler treats this as a compile error, the WebGL 
  compiler accepts it.

- NeuralNetVideoSample had a 'precision mediump float;' declaration, which is
  valid in ESSL but not in GLSL.

It's not really practical to catch all of these cases. Maybe using GLES
instead would give a more compatible result? It's kind of nice to have GL 4.5
though...


ShaderToy JSON Notes
--------------------

**Renderpasses**

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

**Inputs**

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

**Samplers**

- Samplers are specified for each input.
- The ShaderToy website gets a bit confused if you have two inputs that
  reference the same texture but have different sampler settings. It does
  *allow* it though, so I need to support it.

**Outputs**

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
