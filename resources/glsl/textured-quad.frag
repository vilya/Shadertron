#version 450 core

// Fragment shader that renders a textured quad.

uniform sampler2D iChannel0;

in vec2 fUV;

out vec4 oColor;

void main()
{
  oColor = texture(iChannel0, fUV);
}
