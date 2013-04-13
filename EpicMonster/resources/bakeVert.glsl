#extension GL_EXT_gpu_shader4 : require

varying vec4 pos;
varying vec3 normal;
uniform vec3 n;
uniform vec3 offset;
varying vec2 texcoords;

void main()
{
     int vertices = int(n.x)*6;
     pos = gl_Vertex;
     normal = gl_Normal;

     float triwidth = 2.0 / float(n.x);
     float triheight = 2.0 / float(n.x);
     
     int offI = int(offset.z) + gl_VertexID;
     int i = offI % (vertices);
     float row = float(offI / (vertices));
     
     float x = float((i/6) + ((13 >> (5-(i % 6)))&1));
     float y = float((19 >> 5-(i % 6))&1);

     // For debugging individual rendering
     if(n.z < 1.0 || float(gl_VertexID) < n.z)
	 gl_Position = vec4(offset.x+x*triwidth, offset.y-(y+row)*triheight, 0, 1);
     
     texcoords = vec2(gl_MultiTexCoord0);
}

