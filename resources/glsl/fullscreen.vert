#macro GLSL_VERSION

// Vertex shader that generates a full-screen triangle.

void main()
{
  vec4 positions[3];
  positions[0] = vec4(-1.0, -1.0, 0.0, 1.0);
  positions[1] = vec4( 3.0, -1.0, 0.0, 1.0);
  positions[2] = vec4(-1.0,  3.0, 0.0, 1.0);
  gl_Position = positions[gl_VertexID];
}
