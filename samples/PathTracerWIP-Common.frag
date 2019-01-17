//
// Preprocessor definitions
//

// Set this to non-zero to enable a not-very-exciting animation.
#define ANIMATED 0
#define SCENE 0

#if ANIMATED
  #define SAMPLES_PER_PIXEL 4u
  #define SIMULATION_TIME   (iTime * 0.25)
  #define FRAME_BLEND  0.25
#else
  #define SAMPLES_PER_PIXEL 4u
  #define SIMULATION_TIME   1.23
  #define NUM_FRAMES 256.0
  #define FRAME_BLEND (1.0 / 128.0)
#endif // ANIMATED

#define NUM_BOUNCES 12
#define MAX_RAYMARCH_STEPS 128

#define GAMMA 1.8

#define PI        3.141592653589790
#define PI_OVER_2 1.570796326794900
#define PI_OVER_4 0.785398163397448

// The plinth is at the origin, the camera orbits around it.
#if SCENE == 1
  #define CAMERA_DISTANCE	2.0
  #define FOCAL_DISTANCE 	2.0
  #define FOV               radians(90.0)
  #define LENS_RADIUS       0.035
#else
  #define CAMERA_DISTANCE	3.0
  #define FOCAL_DISTANCE 	3.0
  #define FOV               radians(90.0)
  #define LENS_RADIUS       0.035
#endif // SCENE == 1


//
// Structs
//

struct Ray {
    vec3 o; // ray origin
    vec3 d; // ray direction
};


struct Material {
  vec4 baseColor;
  float emission;  //!< 0.0 means no light emitted, higher values mean a brighter light.
  float ior;       //!< Index of refraction.
  float roughness; //!< 1.0 means a very rough surface, 0.0 means a very shiny one.
};


//
// Random number generation
//

uvec4 gRandomState;
uint gRandomStateEx;


uint xorshift32()
{
	uint x = gRandomStateEx;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	gRandomStateEx = x;
	return x;
}


uint xorwow()
{
	uint s = gRandomState.x;
    uint t = gRandomState.w;
    t ^= t >> 2;
    t ^= t << 1;
    t ^= s;
    t ^= (s << 4);
    gRandomState = uvec4(t, gRandomState.xyz);
    gRandomStateEx += 362437u;
    return t + gRandomStateEx;
}


void initRandomState(in vec2 fragCoord, float time)
{
    vec2 tmp = fragCoord;
    gRandomStateEx = (uint(tmp.x + time) & 0xffffu) | ((uint(tmp.y * time) & 0xffffu) << 16);
    gRandomState.x = xorshift32();
    gRandomState.y = xorshift32();
    gRandomState.z = xorshift32();
    gRandomState.w = xorshift32();
}


float random()
{
    uint x = xorwow();
    return float(x) / float(0xffffffffu);
}


vec2 randomVec2()
{
    return vec2(random(), random());
}


vec3 randomVec3()
{
    return vec3(random(), random(), random());
}


vec3 randomRayDir(in vec3 n, in float spread)
{
    return normalize(n + normalize(randomVec3() * 2.0 - 1.0) * spread);
//    return n;
}


//
// Mapping samples to different domains
//

/// Map a point in the unit square to a point on a 
/// disk of unit radius centered at the origin.
vec2 squareToDisk(in vec2 p)
{
    if (p == vec2(0.5)) {
        return vec2(0.0);
    }
    vec2 offset = p * 2.0 - 1.0;
    float theta, r;
    if (abs(offset.x) > abs(offset.y)) {
        r = offset.x;
        theta = PI_OVER_4 * (offset.y / offset.x);
    }
    else {
        r = offset.y;
        theta = PI_OVER_2 - PI_OVER_4 * (offset.x / offset.y);
    }
    return r * vec2(cos(theta), sin(theta));
}
