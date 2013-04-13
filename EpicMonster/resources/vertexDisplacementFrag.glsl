varying vec4 color;

void main(void)
{
    gl_FragColor = vec4(1.0*0.5*abs(color.xyz)+color.w, 1.0);
}