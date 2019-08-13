#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 cubevp[6];

out VertexAttrs {
    vec3 pos;
}vertex;

void main() {
    for(int face = 0; face < 6; ++face) {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i) {
            vertex.pos = gl_in[i].gl_Position.xyz;
            gl_Position = cubevp[face] * gl_in[i].gl_Position;
            EmitVertex();
        }    
        EndPrimitive();
    }
}
