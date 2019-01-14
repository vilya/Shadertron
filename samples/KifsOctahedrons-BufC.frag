// subtle feedback loop: blend the last frame with the current one

vec3 blendSoftLight(vec3 base, vec3 blend) {
    vec3 s = step(0.5,blend);
    return s * (sqrt(base)*(2.0*blend-1.0)+2.0*base*(1.0-blend)) + (1.-s)*(2.*base*blend+base*base*(1.0-2.0*blend));
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
   	vec2 uv = fragCoord.xy / iResolution.xy;
    vec3 base = texture(iChannel0, uv).rgb;
    vec3 overlay = texture(iChannel1, uv).rgb;
    vec4 col = vec4(base +(blendSoftLight(base, overlay*2.)), 1.0);
    
    fragColor = col;
}