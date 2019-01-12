const float MAX_DIST = 10.0;
const float MAX_HEIGHT = 0.3;


vec3 newHalton(int seq)
{
	return vec3(0.0, float(seq), float(seq));	
}


float halton(inout vec3 state)
{
	state.x += 1.0;
	if (state.x == state.y) {
		state *= vec3(0.0, state.z, 1.0);
	}
	return state.x / state.y;
}


vec2 rayOffset(inout vec3 state2, inout vec3 state3)
{
	return vec2(halton(state2), halton(state3)) * 2.0 - 1.0;
}


vec2 pos2uv(in vec3 pos)
{
	return fract(pos.xz / 3.0);
}


float height(in vec2 uv)
{
	float val = textureLod(iChannel0, uv, 0.0).r;
	return val * MAX_HEIGHT;
}


vec3 normal(in vec2 uv)
{
	vec2 eps = 1.0 / iChannelResolution[0].xy;
	vec2 du = vec2(1.0, 0.0) * eps;
	vec2 dv = vec2(0.0, 1.0) * eps;
	return normalize(vec3(height(uv - du) - height(uv + du),
						  (eps.x + eps.y),
						  height(uv - dv) - height(uv + dv)));	
}


vec3 material(in vec2 uv)
{
    return texture(iChannel0, uv).rrr;
}


float castRay(in vec3 pos, in vec3 dir)
{
	const int NUM_SAMPLES = 256;

	float oldT = 0.0;
	float oldH = 0.0;
	float oldY = pos.y;

	for (int i = 0; i < NUM_SAMPLES; ++i) {
		float t = float(i) / float(NUM_SAMPLES);
		t *= t * MAX_DIST;
		vec3 p = pos + dir * t;
		vec2 uv = pos2uv(p);
		float h = height(uv);
		if (p.y < h) {
			return oldT + (t - oldT) * (oldH - oldY) / (p.y - oldY - h + oldH);
		}
		oldT = t;
		oldH = h;
		oldY = p.y;
	}
	return 1e20;
}


vec3 background(in vec3 dir)
{
    return texture(iChannel1, dir).rgb;
}


vec3 lighting(in vec3 N, in vec3 V, in vec3 reflectance)
{
    const float LIGHT_STRENGTH = 1.0;
    
    float NdotV = max(0.0, dot(N, V));
    vec3 diffuse = (texture(iChannel1, N).rgb * LIGHT_STRENGTH) * NdotV;
    
    vec3 L = reflect(-V, N);
    float NdotL = max(0.0, dot(N, L));
    vec3 specular = background(L) * NdotL;
    
    return (specular + diffuse) * reflectance;
}


void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec3 state2 = newHalton(2);
	vec3 state3 = newHalton(3);
	vec3 state5 = newHalton(5);

    vec3 pos = vec3(cos(iTime), 1.2 + sin(iTime) * 0.5, 3.0);
    //vec3 pos = vec3(0.0, 1.2, 3.0);
    vec3 target = vec3(0.0, 0.5, 0.0);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 dir = normalize(target - pos);
    vec3 right = normalize(cross(dir, up));
    up = normalize(cross(right, dir));

    float fov = radians(50.0);
    float imgU = tan(fov) * distance(pos, target);
    float imgV = imgU * iResolution.y / iResolution.x;

    const int RAYS_PER_PIXEL = 5;

    vec3 color = vec3(0.0);
    for (int r = 0; r < RAYS_PER_PIXEL; ++r) {
	    vec2 rayUV = (fragCoord + rayOffset(state2, state3)) / iResolution.xy * 2.0 - 1.0;
	    vec3 rayTarget = target + rayUV.x * imgU * right + rayUV.y * imgV * up;
	    vec3 rayDir = normalize(rayTarget - pos);
	    vec3 rayPos = pos + rayDir * halton(state5) * distance(target, pos) * 0.005;

	    float t = castRay(rayPos, rayDir);
	    if (t <= MAX_DIST) {
	    	vec3 P = pos + rayDir * t;
	    	vec2 UV = pos2uv(P);
	    	vec3 N = normal(UV);
            vec3 reflectance = material(UV);
            color += lighting(N, -rayDir, reflectance);
        }
        else {
            color += background(rayDir);
        }
    }
    color.rgb /= float(RAYS_PER_PIXEL);
    fragColor = vec4(color, 1.0);

    // fragColor = vec4(0.1, 0.2, 0.3, 1.0);
}