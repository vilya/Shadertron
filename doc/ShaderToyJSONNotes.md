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


Samplers
--------

- Samplers are specified for each input.
- The ShaderToy website gets a bit confused if you have two inputs that
  reference the same texture but have different sampler settings. It does
  *allow* it though, so I need to support it.


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
