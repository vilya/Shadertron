#macro GLSL_VERSION

// Vertex shader that generates a textured quad with pixel coordinates
// specified by the `iShape` uniform. Call 
// `glDrawArrays(GL_TRIANGLE_STRIP, 0, 4)` to draw it.

uniform vec3 iResolution;
uniform vec4 iShape; // x, y = bottom left corner, z = width, w = height; all values in pixels.

out vec2 fUV;

void main()
{
  vec2 corners[4];
  corners[0] = vec2(iShape.xy);
  corners[1] = vec2(iShape.x + iShape.z, iShape.y);
  corners[2] = vec2(iShape.x,            iShape.y + iShape.w);
  corners[3] = vec2(iShape.xy + iShape.zw);

  vec2 uv[4];
  uv[0] = vec2(0.0, 0.0);
  uv[1] = vec2(1.0, 0.0);
  uv[2] = vec2(0.0, 1.0);
  uv[3] = vec2(1.0, 1.0);

  gl_Position = vec4(corners[gl_VertexID] / iResolution.xy * 2.0 - 1.0, 0.0, 1.0);
  fUV = uv[gl_VertexID];
}
