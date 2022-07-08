#version 330

// Simple shader to visualize sky model in a fisheye-type view

vec3 sunskyRadiance(vec3 dir);

uniform float exposure = 1;

in vec2 geom_texCoord;

out vec4 fragColor;


float sRGBSingle(float c) {
    float a = 0.055;    
    if (c <= 0) 
        return 0;       
    else if (c < 0.0031308) {
        return 12.92*c;
    } else {
        if (c >= 1.0)
            return 1.0;
        else 
            return (1.0+a)*pow(c, 1.0/2.4)-a;
    }
}

vec4 sRGB(vec4 c) {
    return vec4(sRGBSingle(c.r), sRGBSingle(c.g), sRGBSingle(c.b), c.a);
}

void main() {
    vec2 uv = 2 * geom_texCoord - 1;
    float r = length(uv);
    if (r < 1) {
        vec3 dir = vec3(uv.x, sqrt(1 - r*r), -uv.y);
        fragColor = vec4(exposure * sunskyRadiance(dir), 1.0);
    } else {
        fragColor = vec4(vec3(0.0), 1.0);
    }
}