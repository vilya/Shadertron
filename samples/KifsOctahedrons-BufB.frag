// This buffer does some bokeh based on the alpha channel of buffer A which is the distance of the surface from the pixel.

float normpdf(in float x, in float sigma)
{
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
   	vec2 uv = fragCoord.xy / iResolution.xy;
    vec3 base = vec3(0.);
    
    // Gaussian blur by mrharicot https://www.shadertoy.com/view/XdfGDH
    
    //declare stuff
    const int mSize = 7;
    const int kSize = (mSize-1)/2;
    float kernel[mSize];
    vec3 final_colour = vec3(0.0);
	float depth = texture(iChannel0, uv).g;
    
    //create the 1-D kernel
    float sigma = mix(0.1, 10., max(0., -.55 + depth*0.2));
    float Z = .0;
    for (int j = 0; j <= kSize; ++j)
    {
        kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), sigma);
    }

    //get the normalization factor (as the gaussian has been clamped)
    for (int j = 0; j < mSize; ++j)
    {
        Z += kernel[j];
    }

    //read out the texels
    for (int i=-kSize; i <= kSize; ++i)
    {
        for (int j=-kSize; j <= kSize; ++j)
        {
            base += kernel[kSize+j]*kernel[kSize+i]*texture(iChannel0, (fragCoord.xy+vec2(float(i),float(j))) / iResolution.xy).rgb;
        }
    }
   	vec4 b = vec4(base/(Z*Z), 1.0);

    vec3 col = clamp(vec3(0.),vec3(1.),b.rgb);
 
    fragColor = vec4(col, 1.0);
}
