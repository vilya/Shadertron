Run [ShaderToy](https://www.shadertoy.com) scripts on your local computer,
instead of in a web browser.

The goal is to be 100% compatible with the complete ShaderToy feature set.
This is still in progress; see below for what is & isn't supported so far.

There is direct integration with ShaderToy via their API, allowing you to
download shaders & all their referenced assets directly from the site.

We also provide some useful extensions to ShaderToy's capabilities, such as:

* GLSL code can be stored in external files
* You can edit the code in a text editor of your choice. When you save the
  file, we'll detect the change and automatically reload.
* You can provide your own assets rather than having to use ShaderToy's built
  in ones (although you can use the built-in ones too).


Screenshot
----------

![iq's Surfer Boy shader, running in ShaderTool](doc/Screenshot-SurferBoy.png?raw=true)

This is iq's amazing [Surfer Boy shader](https://www.shadertoy.com/view/ldd3DX), 
running in ShaderTool. The outputs from each of the render passes are shown
along the right of the window and the HUD is showing on the left (both can be
turned off, of course).


ShaderToy Compatibility
-----------------------

*Working*

- Shaders
  - Buf A through Buf D
  - Common
  - Image
- Input sources
  - 2D texture
  - cube map
  - render pass
  - keyboard
  - video
  - webcam
- All standard uniforms
- All input sampler properties (filter mode, wrap mode, srgb/linear and vflip)
- All ShaderToy assets (provided they've been downloaded and placed in the cache - see below)

*Not yet working*

- Shaders
  - Cube A
  - Sound
- Input Sources
  - Audio
  - Microphone
  - SoundCloud
  - Volume texture
- The VR shader entry point.


ShaderToy Integration
---------------------

*Done*

- Download and run shaders directly from ShaderToy, given their ID.
- Assets referenced by downloaded shaders are saved to a local cache.
- There is a an option to download and cache all the standard assets in one
  go (useful if you want to create shaders while offline).

*Still to do*

- Search for shaders on ShaderToy:
  - Support built in sort types: name, love, popular, newest & hot
  - Support built in filter types: vr, soundoutput, soundinput, webcam, multipass, musicstream
- Download and cache thumbnails for shaders.
- Show download progress.


Additional Features
-------------------

- Can toggle keyboard input to shaders on or off (off is useful when you want
  to use keyboard shortcuts, for example).
- Uses ShaderToy's JSON format (as returned by their API) as the native file format, with some minor extensions.
  - Supports external file references
  - Can automatically extract GLSL into external files
  - Can automatically inline GLSL from external files back into the JSON.
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


GLSL vs. ESSL incompatibilities
-------------------------------

There are some language- and driver-level incompatibilities between WebGL and
native GL running locally which can cause valid ShaderToy shaders to fail to
compile in ShaderTool. I've hit these examples so far:

- KifsOctahedron had arguments to `clamp` in the wrong order, leading to
  undefined behaviour. Intel & WebGL shader compilers accept this, the Nvidia
  shader compiler gives incorrect output.

- WordToy used `char` as an identifier, but it's actually a reserved word in 
  GLSL. Nvidia shader compiler treats this as a compile error, the WebGL 
  compiler accepts it.

- NeuralNetVideoSample had a 'precision mediump float;' declaration, which is
  valid in ESSL but not in GLSL.

Other than patching the shader source code at load time, I'm not sure how to
work around these sorts of issues. The simplest option for now is to just
hand-edit any problematic shaders using ShaderTool's output as a guide to what
needs to be fixed.
