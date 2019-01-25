/*
 * "Toon webcam" by Ben Wheatley - 2018
 * License MIT License
 * Contact: github.com/BenWheatley
 */


mat3 k1 = mat3( 1, 2, 1,
                2, 4, 2,
                1, 2, 1)/16.0;

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    float time = iTime;
    vec2 uv = fragCoord.xy / iResolution.xy;
    
    vec2 pixelSize = vec2(1,1) / iResolution.xy;
    
    vec3 sum = vec3(0,0,0);
    
    mat3 kernel = k1;
    
    for (int dy = -1; dy<=1; dy++) {
	    for (int dx = -1; dx<=1; ++dx) {
            vec2 pixelOff = pixelSize * vec2(dx, dy);
            vec2 tex_uv = uv + pixelOff;
            vec3 textureValue = texture(iChannel0, tex_uv).rgb;
            sum += (kernel[dx+1][dy+1] * textureValue);
        }
    }
    
    fragColor = vec4(sum, 1.);
}