// See the Common tab for preprocessor definitions that control various options.

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{   
    vec2 uv = fragCoord / iResolution.xy;
    fragColor = pow(texture(iChannel0, uv), vec4(1.0 / GAMMA));
}
