varying  vec4 color;
varying vec4 lightDiff;


void
main()
{
    gl_FragColor = lightDiff * color;
    
}
