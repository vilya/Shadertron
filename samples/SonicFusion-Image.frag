// Sonic Fusion by Martijn Steinrucken aka BigWings - 2019
// Email:countfrolic@gmail.com Twitter:@The_ArtOfCode
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//
// What you get when you play with the inside of a torus for too long.
// Originally inspired by: https://turtletoy.net/turtle/90e6288a6b
// 
// Turns out that if music is groovy enough, it can cause fusion! ;)
// Use mouse to scrub time.
//
// Music: Daft Punk - Beyond (Nicolas Jaar Feat Dave Harrington Remix)
// https://soundcloud.com/mamulashvili1/daft-punk-beyond-nicolas-jaar-feat-dave-harrington-remix

#define MAX_STEPS 50.
#define SURF_DIST .001
#define SIN(x) (sin(x)*.5+.5)
#define COS(x) (cos(x)*.5+.5)
#define S(x) smoothstep(0.,1.,x)
#define PI 3.1415
#define HPI 1.5708

float bigRadius = 1.;
float smallRadius = .8;

float Fade(float a, float b, float c, float d) {
    float t = mod(iTime,315.); // iChannelTime[0];   // doesn't work on some systems ?
	return smoothstep(a, b, t)*smoothstep(d, c, t);
}

vec3 R(vec2 uv, vec3 p, vec3 l, vec3 up, float z) {
    vec3 f = normalize(l-p),
        r = normalize(cross(up, f)),
        u = cross(f, r),
        c = p+f*z,
        i = c + uv.x*r + uv.y*u,
        d = normalize(i-p);
    return d;
}

mat2 Rot(float a) {
    float s = sin(a);
    float c = cos(a);
    return mat2(c, -s, s, c);
}


float GlowDist(vec3 p) {
    float x = atan(p.x, p.z)+PI;
    float fft = texture(iChannel0, fract(vec2(x/6.2831+.5)), 1./512.).x;
    
    p.y += (fft-.5)*.4;
    
    float s = 1.+sin(x*4.+iTime)*.05;
    float circle = length(vec2(length(p.xz*s)-bigRadius, p.y));
	float d = (circle+smallRadius*.001);
    return d;
}

float D(vec3 p) {
    float circle = length(vec2(length(p.xz)-bigRadius, p.y));
	float d = -(circle-smallRadius);
    return d;
}

vec3 N(vec3 p, float eps) {
	vec2 e = vec2(eps,0);
    return normalize(
        vec3(
           D(p+e.xyy)-D(p-e.xyy),
           D(p+e.yxy)-D(p-e.yxy),
           D(p+e.yyx)-D(p-e.yyx)
        )
    );
}

vec2 TV(vec3 p) {
    float y = atan(length(p.xz)-bigRadius, p.y);
	float x = atan(p.x, p.z);
    return vec2(x, y); 
}
                      
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 m = iMouse.xy/iResolution.xy;
    vec2 UV = fragCoord/iResolution.xy;
	vec2 uv = (UV-.5);
    uv.x *= iResolution.x/iResolution.y;
    
   	float t = iTime*2.-m.x*100.;	// can scrub 50 seconds with mouse
    float t4 = t/4.;
    float t8 = t/8.;
    float t16 = t/16.;
    float t32 = t/32.;
    
    float fft = texture( iChannel0, vec2(.7,0) ).x;
    
    smallRadius = mix(.125, .8, S(SIN(t8)));
    
    // screen twist
    float d = 1.-dot(uv,uv);
    float twist = Fade(0., 100., 200., 300.);
    uv *= Rot(d*2.*twist*sin(t16)*(1.-COS(t32)));
    
    // torus cam setup
    vec3 ro = vec3(0., sin(t8)*.25*smallRadius, -bigRadius-sin(t4)*smallRadius*.3);
    float lookAngle = ((SIN(-t8)));
    vec3 lookDir = vec3(0, 0, 1);
    lookDir.xz *= Rot(lookAngle*HPI);
    float fov =  mix(.5, .1+COS(t32), Fade(30.,60., 250., 300.));      
    vec3 rd = R(uv*Rot(t*.1), ro, ro+lookDir, vec3(0,1,0), fov);
    
    // Ray march
    float dO = 0.;
    float minDistC = 5.;
    for(float i=0.; i<MAX_STEPS; i++) {
    	vec3 p = ro + dO*rd;
        float dS = D(p);
        float dC = GlowDist(p);
        
        minDistC = min(minDistC, dC);
        
        dO += min(dS, dC);
        if(dS<SURF_DIST) break;
    }
    
    vec3 col = vec3(0);
    
    // get pos, normal and torus uvs
    vec3 p = ro + dO*rd;
 	vec3 n = N(p,.01);
    vec2 tv = TV(p)/6.2831+.5;
    
    // Texture
    tv.y += t*.05*Fade(143.5, 143.51, 220.,230.);
    d = fract((tv.y+tv.x)*10.);
    float outline = smoothstep(.3, .28, abs(d-.5));
    float swirl = smoothstep(.25, .2, abs(d-.5));
    float hatchUv = (tv.y-tv.x)*10.*3.1415;
    float hatch = sin(hatchUv*20.+t4)*SIN(t4);
    hatch += sin(hatchUv+t)*SIN(t8);
    col += (outline-swirl*hatch)*.5;
    
    // vignette
    col *= 1.-dot(uv,uv);
    
    // color inversion
    float bw = 1.-clamp(sin(length(uv)*1.-t*.1)*3., 0., 1.);
    col = mix(col, 1.-col, bw);
    
    // add more 3d-ness
    float fresnel = -dot(n, rd);
    fresnel *= fresnel;
    col *= mix(1., fresnel*fresnel, SIN(t8*.5674));
    
    
    // Add some color
    vec3 c = cross(rd, n);
    c.xy *= Rot(t);
    c.zy *= Rot(t4);
    c = c*.5+.5;
    c = pow(c, vec3(1.+SIN(t4)*3.));
    float tasteTheRainbow = Fade(82., 83., 100., 120.);
    tasteTheRainbow += Fade(143.5, 144.5, 170., 200.);
    tasteTheRainbow += Fade(205., 206., 230., 260.);
    col = mix(col, col*c*3., tasteTheRainbow);
    
    // add lightning
    float lightning = fft*(.1/(minDistC));
    vec3 lightningCol = lightning*mix(vec3(1.,.1,.1), vec3(.1, .1, 1.), bw);
    float fade = Fade(21., 25., 50., 70.);
    fade += Fade(82., 83., 100., 120.);
    fade += Fade(143.5, 144.5, 170., 200.);
    fade += Fade(215., 216., 230., 260.);
    
    col += lightningCol * fade;
    
    col *= Fade(0., 10., 290., 315.);
    
    // preview window gets the party right away
    if(iResolution.x<300.) {
       col = col*c*3.+lightningCol;
    }
    
    fragColor = vec4(col,1.0);
}