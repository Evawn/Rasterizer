#version 330

uniform vec3 lightDir;
uniform vec3 k_d;
uniform vec3 k_a;

in vec3 vNormal;

out vec4 fragColor;

void main() {
  vec3 normal = (gl_FrontFacing) ? vNormal : -vNormal;
  float NdotH = max(dot(normalize(normal), normalize(lightDir)), 0.0);
  fragColor = vec4(k_a + NdotH * k_d, 1.0);
}