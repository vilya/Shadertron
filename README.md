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

![iq's Surfer Boy shader, running in Shadertron](doc/Screenshot-SurferBoy.png?raw=true)

This is iq's amazing [Surfer Boy shader](https://www.shadertoy.com/view/ldd3DX), 
running in Shadertron. The outputs from each of the render passes are shown
along the right of the window and the HUD is showing on the left (both can be
turned off, of course).


Features
--------

- Supports a large subset of ShaderToy's feature set.
  - Can already run the majority of shaders verbatim.
  - Working towards 100% support, see below for details.
- Download shaders and assets directly from ShaderToy.
- Live editing of shaders
  - Shaders reload automatically when changed on disk.
- View the results of intermediate renderpasses too
  - Display the output of any image renderpass
  - View the outputs of all renderpasses simultaneously as thumbnails
- Playback controls, including fast forward and rewind
- Pan and zoom the shader output
- Choice of rendering resolutions, doesn't have to match the window size (very useful for slow shaders!)
- Save screenshots
- Save the output of intermediate renderpasses to an image file


ShaderToy Compatibility
-----------------------

- [ ] Shaders
  - [x] Buf A through Buf D
  - [x] Common
  - [x] Image
  - [x] Cube A
  - [ ] Sound
- [x] Input sources
  - [x] 2D texture
  - [x] Cube map
  - [x] Render pass
  - [x] Keyboard
  - [x] Video
  - [x] Webcam
  - [ ] Audio
  - [ ] Microphone
  - [ ] SoundCloud
  - [ ] Volume texture
- [x] All standard uniforms
- [x] All input sampler properties (filter mode, wrap mode, srgb/linear and vflip)
- [x] All ShaderToy assets (provided they've been downloaded and placed in the cache - see below)
- [ ] The VR shader entry point


ShaderToy Integration
---------------------

- [x] Download and run shaders directly from ShaderToy given their ID.
- [x] Assets referenced by downloaded shaders are saved to a local cache.
- [x] There is a an option to download and cache all the standard assets in one go (useful if you want to create shaders while offline).
- [ ] Search for shaders on ShaderToy:
  - [ ] Support built in sort types: name, love, popular, newest & hot
  - [ ] Support built in filter types: vr, soundoutput, soundinput, webcam, multipass, musicstream
- [x] Download and cache thumbnails for shaders.
- [ ] Display thumbnails for shaders in the search results.
- [ ] Show download progress.


Video support
-------------

Some of ShaderToy's videos are in .ogv format & others are in .webm.
Shadertron uses whatever video codecs are installed on your system; you may
need to install additional codecs to be able to play back these formats.

On Windows, for more information about codecs, including how to find out which
ones are available on your system, see:

https://support.microsoft.com/en-gb/help/15070/windows-media-player-codecs-frequently-asked-questions

The K-Lite Codec Pack includes support for .ogv and .webm as well as a variety
of other video formats. It's regularly updated and seems trustworthy. It's
available from here:

http://www.codecguide.com/about_kl.htm

I don't know what the equivalent is for macOS or Linux yet.


GLSL vs. ESSL incompatibilities
-------------------------------

There are some language- and driver-level incompatibilities between WebGL and
native GL running locally which can cause valid ShaderToy shaders to fail to
compile in Shadertron. I've hit these examples so far:

- KifsOctahedron had arguments to `clamp` in the wrong order, leading to
  undefined behaviour. Intel & WebGL shader compilers accept this, the Nvidia
  shader compiler gives incorrect output.

- WordToy used `char` as an identifier, but it's actually a reserved word in 
  GLSL. Nvidia shader compiler treats this as a compile error, the WebGL 
  compiler accepts it.

- NeuralNetVideoSample had a 'precision mediump float;' declaration, which is
  valid in ESSL but not in GLSL.

- CircularPuzzleInteractive has a ternary-if with assignments in the then and
  else branches. Assignment in the `then` branch seems to to be handled fine, 
  but assignment in the `else` branch seems to confuse the native GLSL 
  compiler. Putting parentheses around this assignment fixes the issue.

Other than parsing & automatically patching shader source code at load time,
I'm not sure how to work around these sorts of issues. The simplest option for
now is to just hand-edit any problematic shaders using Shadertron's output as
a guide to what needs to be fixed.
