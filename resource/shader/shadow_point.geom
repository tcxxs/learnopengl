#version 460 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 18) out;

uniform mat4 light_vp[6];

in VertexAttrs {
    vec2 uv;
}vertex_in[];

out VertexAttrs {
    vec3 pos;
    vec2 uv;
}vertex_out;

void main() {
    for(int face = 0; face < 6; ++face) {
        gl_Layer = face;
        for(int i = 0; i < 3; ++i) {
            vertex_out.pos = gl_in[i].gl_Position.xyz;
            vertex_out.uv = vertex_in[i].uv;
            gl_Position = light_vp[face] * gl_in[i].gl_Position;
            EmitVertex();
        }    
        EndPrimitive();
    }
}
