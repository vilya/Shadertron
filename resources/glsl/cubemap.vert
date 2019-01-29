#macro GLSL_VERSION

// Vertex shader that generates a "twonit" cube (i.e. a cube from -1 to +1
// along each axis).

uniform vec3 iRayDirs[3];

out vec3 fRayDir;


void main()
{
  vec4 positions[3];
  positions[0] = vec4(-1.0, -1.0, 0.0, 1.0);
  positions[1] = vec4( 3.0, -1.0, 0.0, 1.0);
  positions[2] = vec4(-1.0,  3.0, 0.0, 1.0);
  gl_Position = positions[gl_VertexID];
  fRayDir = iRayDirs[gl_VertexID];
}
