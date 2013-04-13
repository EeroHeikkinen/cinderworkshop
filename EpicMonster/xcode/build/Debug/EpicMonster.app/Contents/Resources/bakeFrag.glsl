//#extension GL_ARB_draw_buffers : enable
//#extension GL_ARB_texture_rectangle : enable
//#extension GL_ARB_texture_non_power_of_two : enable

#define EPS 1.1

uniform sampler2D positions;
uniform sampler2D velocities;
uniform sampler2D normalMap;
uniform float scale;
varying vec4 pos;
varying vec3 normal;
varying vec2 texcoords;

void main(void)
{   
    vec3 bump = vec3(texture2D(normalMap, texcoords));
    vec4 pos1 = pos + scale * vec4(normal * bump,1);
    //Render to positions texture
    gl_FragData[0] = vec4(pos1);
    //Render to velocities texture
    gl_FragData[1] = vec4(0,0,0, 1.0);
}