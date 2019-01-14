// Comment this out to have the parameters that generated the gif
#define USE_MOUSE

// HG_SDF
// Brought to you by MERCURY http://mercury.sexy
// Released as Creative Commons Attribution-NonCommercial (CC BY-NC)

#define TAU (2*PI)
#define PHI (1.618033988749895)
#define GDFVector0 vec3(1, 0, 0)
#define GDFVector1 vec3(0, 1, 0)
#define GDFVector2 vec3(0, 0, 1)

#define GDFVector3 normalize(vec3(1, 1, 1 ))
#define GDFVector4 normalize(vec3(-1, 1, 1))
#define GDFVector5 normalize(vec3(1, -1, 1))
#define GDFVector6 normalize(vec3(1, 1, -1))

#define fGDFBegin float d = 0.;

// Version with without exponent, creates objects with sharp edges and flat faces
#define fGDF(v) d = max(d, abs(dot(p, v)));

#define fGDFExpEnd return pow(d, 1./e) - r;
#define fGDFEnd return d - r;

float fOctahedron(vec3 p, float r) {
	fGDFBegin
    fGDF(GDFVector3) fGDF(GDFVector4) fGDF(GDFVector5) fGDF(GDFVector6)
    fGDFEnd
}

void pR(inout vec2 p, float a) {
	p = cos(a)*p + sin(a)*vec2(p.y, -p.x);
}

// end HG_SDF
//----------------------------------------------------------------------

float map( in vec3 pos ){
    float scale = .55;

    vec2 spacesize = vec2(3.65,2.75);
    vec2 idx = floor(pos.xz/spacesize);
    pos.xz = mod(pos.xz, spacesize) - spacesize*0.5;
    
    vec3 p = pos;
    
    vec2 rot;
    #ifdef USE_MOUSE
    rot = vec2(0.785,2.412)+3.14*iMouse.xy/iResolution.xy;
    #else
    rot = vec2(0.785,2.412);
    #endif
    
	float clock = iTime*2.;
	vec2 rotAnimPhase = vec2(clock + idx.x, clock + idx.y);
	vec2 rotAnimAmp = vec2(0.15, 0.3);
    float cube = 1e20;
   
    vec3 displacement = vec3(-1.125, -0.75, -0.375);
    
    for (int i = 0; i < 8; i++) {
        p.xyz = abs(p.xyz);
        p += displacement * scale;
		
        pR(p.xy, rot.x+sin(rotAnimPhase.x+ float(i)*0.5 + length(p)*.5 + idx.x*2.)*rotAnimAmp.x);
        pR(p.yz, rot.y+cos(rotAnimPhase.y+ float(i)*0.5 + length(p)*.5 + idx.y*2.)*rotAnimAmp.y);

		scale *= 0.6;
        
        float octa = fOctahedron(p,scale);
      
        cube = min(cube,octa);    
        
    }

    return cube;
}

float castRay( in vec3 ro, in vec3 rd )
{
	float precis = 0.007;
    float t = 0.;
    float m = 0.0;
    for( int i=0; i<40; i++ )
    {
   		float res = map( ro+rd*t );
        if (res < precis) break;
        t += res;
    }

    return t;
}

vec3 calcNormal( in vec3 pos )
{
	vec3 eps = vec3( 0.005, 0.0, 0.0 );
	vec3 nor = vec3(map(pos+eps.xyy) - map(pos-eps.xyy),
        			map(pos+eps.yxy) - map(pos-eps.yxy),
        			map(pos+eps.yyx) - map(pos-eps.yyx) );
	return normalize(nor);
}

float calcAO( in vec3 pos, in vec3 nor )
{
	float occ = 0.0;
    float sca = 1.0;
    for( int i=0; i<4; i++ )
    {
        float hr = 0.01 + 0.02*float(i)/4.0;
        vec3 aopos =  nor * hr + pos;
        float dd = map( aopos );
        occ += -(dd-hr)*sca;
        sca *= .95;
    }
    return clamp( 1.0 - 3.0*occ, 0.0, 1.0 );    
}

vec4 render( in vec3 ro, in vec3 rd )
{
    vec3 col = vec3(1.);
    float res = castRay(ro,rd);
    vec3 pos = ro + res*rd;
    vec3 nor = calcNormal( pos );

    float occ = calcAO( pos, nor )*1.25;

    col = vec3(.85+dot(rd,nor));
	nor.g = 0.;
    col *= (1.-nor.rgb*0.5-0.5) * occ;
	
    col = mix(col, vec3(0.), clamp((res)/6.5, 0., 1.));

    return vec4( clamp(col,0.0,1.0), res );
}

mat3 setCamera( in vec3 ro, in vec3 ta, float cr )
{
    vec3 cw = normalize(ta-ro);
    vec3 cp = vec3(sin(cr), cos(cr),0.0);
    vec3 cu = normalize( cross(cw,cp) );
    vec3 cv = normalize( cross(cu,cw) );
    return mat3( cu, cv, cw );
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) 
{
    vec2 q = fragCoord.xy/iResolution.xy;
    vec2 p = -1.0+2.0*q;
    p.x *= iResolution.x/iResolution.y;

    vec3 ro = vec3(-0.65 , 2., -1.5);
    vec3 ta = ro+vec3(.45+sin(iTime*2.)*0.025,-.475 + cos(1.5+iTime*2.)*0.025,1.);
    mat3 ca = setCamera( ro, ta, 0. );
    vec3 rd = ca * normalize(vec3(p.xy,3.4 + cos(iTime*2.)*0.15));
    vec4 col = render( ro, rd );

    fragColor=col;
}