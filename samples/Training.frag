void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
  vec2 p = fragCoord.xy / iResolution.xy;
  float x = fract((p.x + iTime) * .007) * 100.,   // landscape is a function of this x value
      	d = .5 - length(p - .5),                        // distance from ellipse, for the vignette
      	r = .3,                                        	// fade-out width for the vignette
      	h = .7 + sin(x) * .1 + sin(x * 21.1 + 3.) * .03 + cos(x * 49.8 + 1.3) * .01, // height of the mountains
        g = .3 + fract(sin(x * 12.9) * 43758.5) * .05,	// height of the grass
        c = pow(p.y + .2, .8); 								// base color
  if (p.y < h)
    c -= .3; // color the mountains
  if (p.y < g)
    c -= .2; // color the grass
  fragColor = c * smoothstep(0., r, d + r) * vec4(.3, .4, 1, 1);
}
