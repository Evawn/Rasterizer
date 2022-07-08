#version 330

const float PI = 3.14159265358979323846264;
const float INV_SQRT_TWOPI = 1.0 / sqrt(2 * PI);

uniform sampler2D image;

uniform float stdev = 6;
uniform int radius = 8;
uniform vec2 dir = vec2(1.0, 0.0);
uniform int level = 0;

in vec2 geom_texCoord;

out vec4 fragColor;

float gaussianWeight(int r) {
    return exp(-r * r / (2.0 * stdev * stdev)) * INV_SQRT_TWOPI / stdev;
}

void main() {
    vec2 d = 1.0 / textureSize(image, level);
    vec3 s = vec3(0);
    for(int i = -radius; i <= radius; i++) {
        float w = gaussianWeight(i);
        s += w * textureLod(image, geom_texCoord + d * i * dir, level).rgb;
    }
    fragColor = vec4(s, 1.0);
}