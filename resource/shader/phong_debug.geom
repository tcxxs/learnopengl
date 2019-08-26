#version 460 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VertexAttrs {
    vec3 pos;
    vec2 uv;
    vec3 normal;
}vertex_in[];

out VertexAttrs {
    vec3 pos;
    vec2 uv;
    vec3 normal;
}vertex_out;

void explode() {
    vec3 normal = mix(vertex_in[0].normal, mix(vertex_in[1].normal, vertex_in[2].normal, 0.5), 0.5);
    vec4 offset = vec4(normalize(normal) * 0.05, 0.0);
    for (int i = 0; i < 3; ++i) {
        vertex_out.pos = vertex_in[i].pos;
        vertex_out.uv = vertex_in[i].uv;
        vertex_out.normal = vertex_in[i].normal;
        gl_Position = gl_in[i].gl_Position + offset;
        EmitVertex();
    }
    EndPrimitive();
}

void normals() {
    for (int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
        gl_Position = gl_in[i].gl_Position + vec4(normalize(vertex_in[i].normal) * 0.01, 0.0);
        EmitVertex();
        EndPrimitive();
    }
}

void main() {
    normals();
}