// See the Common tab for preprocessor definitions that control various options.

// TODO: refraction is buggy, doesn't handle glass bubbles (i.e. a hollow glass
// ball) correctly. Set SCENE == 1 for a debug scene (copied from Peter Shirley's
// "Raytracing in One Weekend" book) that shows this problem.


//
// Constants
// 

#if SCENE == 1
  #define MAT_BACKGROUND   0.0
  #define MAT_GROUND       1.0
  #define MAT_GLASS_SPHERE 2.0
  #define MAT_SOLID_SPHERE 3.0
  #define MAT_METAL_SPHERE 4.0
#else
  #define MAT_BACKGROUND   0.0
  #define MAT_LIGHT_SPHERE 1.0
  #define MAT_GLASS_SPHERE 2.0
  #define MAT_GROUND       3.0
  #define MAT_PLINTH       4.0
#endif


//
// Globals
//

float gCosTime;
float gSinTime;
vec3 gLightPos;
mat3 gSphereRot;


//
// Signed distance functions
// 

float opSub(float d0, float d1)
{
  return max(d0, -d1);
}


float sdSphere(in vec3 p, in vec3 center, in float radius)
{
  return distance(p, center) - radius;
}


float sdBox(in vec3 p, in vec3 b)
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}


float sdGround(in vec3 p)
{
//  return p.y;
  vec2 uv = p.xz * 0.5;
  return max(p.y - texture(iChannel1, uv).r * 0.2, -1e-5) * 0.33;
}


float sdPlinth(in vec3 p)
{
  return opSub(sdBox(p - vec3(0.0, 1.0, 0.0), vec3(0.5, 1.0, 0.5)),
               sdBox(p - vec3(0.0, 0.8, 0.0), vec3(0.3, 0.8, 0.7)));
}


//
// Model the scene
//

void sceneInit()
{
  gCosTime = cos(SIMULATION_TIME);
  gSinTime = sin(SIMULATION_TIME);

  gLightPos = vec3(-gCosTime, 0.25, gSinTime);
  gLightPos.y += gLightPos.y - sdGround(gLightPos);
  
  gSphereRot = mat3(gCosTime, 0, -gSinTime,
                    0,        1, 0,
                    gSinTime, 0, gCosTime);
}



// Return value is { min distance, material index }
vec2 scene(in vec3 sp)
{
#if SCENE == 1
    
  // Initialize with distance to the ground plane.
  vec2 result = vec2(sdSphere(sp, vec3(0.0, -100.0, 0.0), 100.0), MAT_GROUND);
    
  float d = sdSphere(sp, vec3(0.0, 0.5, 0.0), 0.5);
  if (d < result.x) {
    result = vec2(d, MAT_SOLID_SPHERE);
  }

//  d = sdSphere(sp, vec3(-1.0, 0.5, 0.0), 0.5);
  d = opSub(sdSphere(sp, vec3(-1.0, 0.5, 0.0), 0.5),
            sdSphere(sp, vec3(-1.0, 0.5, 0.0), 0.49));
  if (d < result.x) {
    result = vec2(d, MAT_GLASS_SPHERE);
  }

  d = sdSphere(sp, vec3(1.0, 0.5, 0.0), 0.5);
  if (d < result.x) {
    result = vec2(d, MAT_METAL_SPHERE);
  }
    
#else
    
  // Initialize with distance to the ground plane.
  vec2 result = vec2(sdGround(sp), MAT_GROUND);
  
  // Light-emitting sphere.
  float d = sdSphere(sp, gLightPos, 0.25);
  if (d < result.x) {
    result = vec2(d, MAT_LIGHT_SPHERE);
  }

  // Glass spheres
  vec3 p = clamp(gSphereRot * sp, vec3(-3.0), vec3(3.0)); // Clamp and mod `sp` to make a 3x1x3 grid of spheres
  p = mod(p + 1.0, 2.0) - 1.0;
  p.y = sp.y;
  d = opSub(sdSphere(p, vec3(0.0, 0.5, 0.0), 0.45),
            sdSphere(p, vec3(0.0, 0.5, 0.0), 0.40));
  d = opSub(d, sdSphere(sp, vec3(0.0, 0.5, 0.0), 0.55));
  if (d < result.x) {
    result = vec2(d, MAT_GLASS_SPHERE);
  }
  
  // Plinth
  d = sdPlinth(sp);
  if (d < result.x) {
    result = vec2(d, MAT_PLINTH);
  }
    
#endif
    
  return result;
}


Material material(in float matIdx, in vec3 p, in vec3 rd)
{
    // Material properties are:
    // - base color (vec4, containing RGBA values)
    // - emission
    // - index of refraction
    // - roughness
#if SCENE == 1
    
    if (matIdx == MAT_GLASS_SPHERE) {
        return Material(vec4(1.0, 1.0, 1.0, 0.0), 0.0, 1.5, 0.0);
    }
    else if (matIdx == MAT_SOLID_SPHERE) {
        return Material(vec4(0.1, 0.2, 0.5, 1.0), 0.0, 1.0, 0.25);
    }
    else if (matIdx == MAT_METAL_SPHERE) {
        return Material(vec4(0.8, 0.6, 0.2, 1.0), 0.0, 1.0, 0.0);
        //return Material(vec4(1.0, 1.0, 1.0, 0.0), 0.0, 1.5, 0.0);
    }
    else if (matIdx == MAT_GROUND) {
  		return Material(vec4(0.8, 0.8, 0.0, 1.0), 0.0, 1.0, 0.9);
    }
    else { // background
        vec4 color = vec4(mix(vec3(1.0), vec3(0.5, 0.7, 1.0), rd.y * 0.5 + 0.5), 1.0);
        return Material(color, 1.0, 1.0, 0.0);
    }
    
#else
    
    if (matIdx == MAT_LIGHT_SPHERE) {
        return Material(vec4(0.4, 0.6, 0.4, 1.0), 16.0, 1.0, 0.5);
    }
    else if (matIdx == MAT_GLASS_SPHERE) {
        return Material(vec4(1.0, 1.0, 1.0, 0.0), 0.0, 1.5, 0.01);
    }        
    else if (matIdx == MAT_GROUND) {
        vec2 uv = p.xz * 0.5;
        vec4 color = vec4(texture(iChannel1, uv).rrr * vec3(0.6, 0.55, 0.53), 1.0);
  		return Material(color, 0.0, 1.0, 1.0);
    }
    else if (matIdx == MAT_PLINTH) {
  		return Material(vec4(0.3, 0.3, 0.8, 0.5), 0.0, 1.778, 0.0);
    }
    else { // Background (skybox)
        //vec4 color = vec4(1.0, 1.0, 0.5, 1.0);
        vec4 color = texture(iChannel0, rd);
        return Material(color, 0.67, 1.0, 0.0);
    }
    
#endif // SCENE == 0
}


//
// Mini raymarcher
//

// Use this to get a ray dir vector for your camera.
Ray lookAt(in vec3 pos, 
           in vec3 target, 
           in vec3 up, 
           in float fov, 
           in float focalDistance,
           in vec2 lensCoord,
           in vec2 fragCoord)
{
  vec2 uv = (fragCoord.xy / iResolution.xy) * 2.0 - 1.0;
  float imgU = atan(fov) * distance(pos, target);
  float imgV = imgU * iResolution.y / iResolution.x;
  vec3 forward = normalize(target - pos);
  vec3 right = normalize(cross(forward, up));
  vec3 orthogonalUp = normalize(cross(right, forward));

  vec3 focalPoint = pos + forward * focalDistance;
    
  Ray r;
  r.o = pos + lensCoord.x * right + lensCoord.y * orthogonalUp;
  r.d = normalize(focalPoint + right * imgU * uv.x + orthogonalUp * imgV * uv.y - r.o);
  return r;
}


// Calculate the normal at an arbitrary point in the scene.
vec3 normal(in vec3 p)
{
  float epsilon = 1e-5;
  vec3 dx = vec3(epsilon, 0.0, 0.0);
  vec3 dy = vec3(0.0, epsilon, 0.0);
  vec3 dz = vec3(0.0, 0.0, epsilon);
  return normalize(vec3( scene(p + dx).x - scene(p - dx).x,
                         scene(p + dy).x - scene(p - dy).x,
                         scene(p + dz).x - scene(p - dz).x ));
}


// Find how far along the ray until the next object intersection occurs, in
// the range `minT` to `maxT`.
//
// Return value is a vec2 with .x = t value, .y = material index.
vec2 raymarch(in vec3 ro, in vec3 rd, in float minT, in float maxT)
{
  float t = minT;
  for (int i = 0; i < MAX_RAYMARCH_STEPS; ++i) {
    vec2 result = scene(ro + rd * t);
    if (abs(result.x) <= 1e-5) {
      return vec2(t, result.y);
    }
    t += abs(result.x);
    if (t >= maxT) {
      break;
    }
  }
  return vec2(maxT, 0.0);
}


float fresnelReflectance(float cosTheta, float srcIOR, float dstIOR)
{
    // Using Schlick's approximation, as per https://en.wikipedia.org/wiki/Schlick%27s_approximation
    float r0 = (srcIOR - dstIOR) / (srcIOR + dstIOR);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow(1.0 - cosTheta, 5.0);
}


bool brdf(in vec3 N, in Material mat, inout vec3 rd, inout vec3 transmission)
{
    rd = reflect(rd, N);
    if (mat.roughness > 0.0) {
        vec3 roughDir = randomRayDir(rd, mat.roughness);

        // If `roughDir` is a zero-length vector, then just ignore it.
        if (dot(roughDir, roughDir) < 0.5) {
            return true;
        }

        rd = roughDir;
        
        // If `roughDir` is in the negative hemisphere, project it onto the 
        // base of the positive hemisphere so that it's perpendicular to the 
        // normal.
        float rd_dot_N = dot(rd, N);
        if (rd_dot_N < 0.0) {
            rd = rd - rd_dot_N * N;
        }
    }
    return true;
}


bool btdf(in vec3 N, in Material mat, in vec2 ior, inout vec3 rd, inout vec3 transmission)
{
    vec3 refracted = refract(rd, N, ior[0] / ior[1]);
    if (dot(refracted, refracted) < 0.5) {
        if (brdf(N, mat, rd, transmission)) {
            //transmission *= mat.baseColor.rgb * mat.baseColor.a;
            return true;
        }
        return false;
    }
    
    rd = normalize(refracted);
    
    if (mat.roughness > 0.0) {
        vec3 roughDir = randomRayDir(rd, mat.roughness);

        // If `roughDir` is a zero-length vector, then just ignore it.
        if (dot(roughDir, roughDir) < 0.5) {
            return true;
        }

        rd = roughDir;
        
        // If `roughDir` is in the positive hemisphere, project it onto the 
        // base of the negative hemisphere so that it's perpendicular to the 
        // normal.
        float rd_dot_N = dot(rd, N);
        if (rd_dot_N > 0.0) {
            rd = rd - rd_dot_N * N;
            //return false;
        }
    }

    transmission *= mat.baseColor.rgb * (1.0 - mat.baseColor.a);
    return true;
}


bool scatter(in Material mat,
             in vec3 ro, 
             inout vec3 rd, 
             inout vec3 transmission)
{
    vec3 N = normal(ro);
    float rd_dot_N = dot(rd, N);
    bool leaving = (rd_dot_N > 0.0);
    if (leaving) {
        N = -N;
        rd_dot_N = -rd_dot_N;
    }
    vec2 ior = leaving ? vec2(mat.ior, 1.0) : vec2(1.0, mat.ior); // x = src IOR, y = dst IOR

    // Check for specular reflection.
    //
    // Specular reflection currently leaves `transmission` unchanged, i.e. 
    // all incoming light is reflected.
    if (random() < fresnelReflectance(abs(rd_dot_N), ior[0], ior[1])) {
        return brdf(N, mat, rd, transmission);
    }
    
    // Check for refraction.
    if (mat.baseColor.a < 1.0 && random() >= mat.baseColor.a) {
        return btdf(N, mat, ior, rd, transmission);
    }

    // Reflect.
    if (brdf(N, mat, rd, transmission)) {
        transmission *= mat.baseColor.rgb * mat.baseColor.a;
        return true;
    }
    return false;
}


vec3 irradiance(in vec3 ro, inout vec3 rd)
{
    float maxT = 8.0;
    vec3 transmission = vec3(1.0);
    vec3 lightColor = vec3(0.0, 0.0, 0.0);
   
    for (int numBounces = 0; numBounces < NUM_BOUNCES; numBounces++) {
      vec2 hit = raymarch(ro, rd, 5e-4, maxT);
        
      ro += rd * hit.x;

      Material mat = material(hit.y, ro, rd);
    
      // If we've reached something that emits light, stop tracing here and transmit 
      // the light.
      if (mat.emission > 0.0) {
          lightColor = mat.baseColor.rgb * mat.emission;
          break;
      }

      // Try to scatter the ray. If it gets absorbed, then we can end the loop early.
      if (!scatter(mat, ro, rd, transmission)) {
        break;
      }      
        
      // Also end the loop if the amount of light that will be transmitted is too 
      // small to make a difference to the final image.
      if (dot(transmission, transmission) < 1e-5) {
        break;
      }
    }

    return transmission * lightColor;
}


void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    float framesLeft = texelFetch(iChannel2, ivec2(0, 0), 0).w;
    bool cameraMoved = iMouse.z >= 0.0 || iMouse.w >= 0.0 || iFrame == 0;
    if (cameraMoved) {
        framesLeft = NUM_FRAMES;
    }
    bool isDataPixel = fragCoord == vec2(0.5);
    float w = isDataPixel ? (framesLeft - 1.0) : 1.0;
    if (framesLeft <= 0.0) {
        fragColor = vec4(texture(iChannel2, fragCoord / iResolution.xy).rgb, 0.0);
        return;
    }

    initRandomState(fragCoord, iTime);
	  sceneInit();
    
    vec2 mouseUV = iMouse.xy / iResolution.xy;
    vec3 camPos = vec3((vec2(0.5, 1.0) - mouseUV) * 5.0, CAMERA_DISTANCE);

    vec3 color = vec3(0.0);
    for (uint sampleNum = 0u; sampleNum < SAMPLES_PER_PIXEL; sampleNum++) {
      vec2 jitteredFragCoord = fragCoord + randomVec2();
      vec2 jitteredLensCoord = squareToDisk(randomVec2()) * LENS_RADIUS;
	    Ray r = lookAt(camPos, vec3(0.0), vec3(0.0, 1.0, 0.0), FOV, FOCAL_DISTANCE, jitteredLensCoord, jitteredFragCoord);
      color += irradiance(r.o, r.d);
    }
    color /= float(SAMPLES_PER_PIXEL);
    
    vec3 prevColor = texture(iChannel2, fragCoord / iResolution.xy).rgb;
    
  #if ANIMATED
    color = mix(prevColor, color, FRAME_BLEND);
  #else
    if (!cameraMoved) {
	    color = mix(prevColor, color, FRAME_BLEND);
    }
  #endif
            
    fragColor = vec4(color, w);
}
