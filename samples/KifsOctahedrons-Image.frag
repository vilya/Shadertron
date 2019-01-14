void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord/iResolution.xy;
    vec2 uuv = uv * 2.0 - 1.0;
    vec3 c = texture(iChannel0, uv).rgb;
    vec4 col = vec4(pow(c, vec3(0.5)), 1.0);

    fragColor = mix(vec4(.0), col, 1.-smoothstep(0.,1.,length(uuv)*0.6));
}