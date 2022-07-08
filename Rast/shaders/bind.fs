#version 330

//uniform vec3 lightDir;
uniform vec3 k_d;
// uniform vec3 k_a;
uniform float alpha;
uniform float eta;
// uniform vec3 lightPos;
// uniform vec3 power;
uniform mat4 mV;  // View matrix
uniform mat4 mP;  // Perspective matrix

in vec3 vNormal;
in vec3 vPosition;

layout(location = 0) out vec4 diffuse;
layout(location = 1) out vec4 specular;
layout(location = 2) out vec4 normals;

void main() {

    vec3 normal = (gl_FrontFacing) ? vNormal : -vNormal;
    normal = normalize(normal) / 2 + 0.5;

    diffuse = vec4(k_d, 0);
    specular = vec4(alpha, eta/100, 0, 0);
    normals = vec4(normal, 0);

}