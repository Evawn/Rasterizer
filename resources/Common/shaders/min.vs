#version 330

uniform mat4 mM;  // Model matrix
uniform mat4 mV;  // View matrix
uniform mat4 mP;  // Projection matrix

layout (location = 0) in vec3 position;

out vec3 vPosition;  // vertex position in eye space

void main()
{
    vPosition = (mV * mM * vec4(position, 1.0)).xyz;
    gl_Position = mP * vec4(vPosition, 1.0);
}
