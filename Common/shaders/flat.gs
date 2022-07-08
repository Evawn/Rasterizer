#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec3 vPosition[];  // in eye space

out vec3 vNormal;   // in eye space

void main() {
    vNormal = normalize(cross(vPosition[1] - vPosition[0], vPosition[2] - vPosition[0]));
    for (int i = 0; i < gl_in.length(); i++) {
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}