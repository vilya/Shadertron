void mainCubemap(out vec4 fragColor, in vec2 fragCoord, in vec3 rayOri, in vec3 rayDir)
{
    fragColor = vec4(rayDir * 0.5 + 0.5, 1.0);
}