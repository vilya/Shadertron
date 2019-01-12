#version 450 core

// Vertex shader that generates a full-screen triangle.
uniform vec3 iResolution;

out vec2 ShaderToolQt_fFragCoord;

void main()
{
  vec4 positions[3];
  positions[0] = vec4(-1.0, -1.0, 0.0, 1.0);
  positions[1] = vec4( 3.0, -1.0, 0.0, 1.0);
  positions[2] = vec4(-1.0,  3.0, 0.0, 1.0);
  gl_Position = positions[gl_VertexID];

  vec2 fragCoords[3];
  fragCoords[0] = vec2(0.0,                 0.0          );
  fragCoords[1] = vec2(iResolution.x * 2.0, 0.0          );
  fragCoords[2] = vec2(0.0,                 iResolution.y * 2.0);
  ShaderToolQt_fFragCoord = fragCoords[gl_VertexID];
}
