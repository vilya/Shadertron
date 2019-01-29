#define POS_X 0
#define NEG_X 1
#define POS_Y 2
#define NEG_Y 3
#define POS_Z 4
#define NEG_Z 5
#define NO_FACE -1


void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
  vec2 uv = (fragCoord / iResolution.xy) * vec2(4.0, 3.0) - vec2(0.0, 1.0);

  int face = NO_FACE;
  if (uv.y >= 0.0 && uv.y < 1.0) {
    if (uv.x < 1.0) {
      face = NEG_X;
    }
    else if (uv.x < 2.0) {
      face = NEG_Z;
    }
    else if (uv.x < 3.0) {
      face = POS_X;
    }
    else {
      face = POS_Z;
    }
  }
  else if (uv.x >= 1.0 && uv.x < 2.0) {
    if (uv.y >= 1.0) {
      face = POS_Y;
    }
    else if (uv.y < 0.0) {
      face = NEG_Y;
    }
  }

  if (face == NO_FACE) {
    fragColor = vec4(0.0);
    return;
  }

  uv = fract(uv) * 2.0 - 1.0;

  vec3 rayDir;
  if (face == NEG_Z) {
    rayDir = vec3(uv.x, uv.y, -1.0);
  }
  else if (face == POS_Z) {
    rayDir = vec3(-uv.x, uv.y, 1.0);
  }
  else if (face == NEG_X) {
    rayDir = vec3(-1.0, uv.y, -uv.x);
  }
  else if (face == POS_X) {
    rayDir = vec3(1.0, uv.y, uv.x);
  }
  else if (face == NEG_Y) {
    rayDir = vec3(uv.x, -1.0, -uv.y);
  }
  else if (face == POS_Y) {
    rayDir = vec3(uv.x, 1.0, uv.y);
  }
  else {
    rayDir = vec3(0.0, 0.0, -1.0);
  }

  rayDir = normalize(rayDir);

  if (iMouse.z >= 0.0 && iMouse.w >= 0.0) {
    fragColor = vec4(rayDir * 0.5 + 0.5, 1.0);
  }
  else {
    fragColor = texture(iChannel0, rayDir);
  }
}
