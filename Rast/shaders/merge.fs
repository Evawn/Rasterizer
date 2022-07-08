#version 330

// const float PI = 3.14159265358979323846264;
// const float INV_SQRT_TWOPI = 1.0 / sqrt(2 * PI);

uniform sampler2D mipmap;
uniform sampler2D original;

uniform vec4 levels;

in vec2 geom_texCoord;

out vec4 fragColor;


void main() {
   
    vec4 sample1 = textureLod(mipmap, geom_texCoord, levels.x);
    vec4 sample2 = textureLod(mipmap, geom_texCoord, levels.y);
    vec4 sample3 = textureLod(mipmap, geom_texCoord, levels.z);
    vec4 sample4 = textureLod(mipmap, geom_texCoord, levels.w);
    vec4 orig = texture(original, geom_texCoord);

    float a_0 = 0.8843;
    float a_1 = 0.1;
    float a_2 =0.012;
    float a_3 =0.0027;
    float a_4 =0.001;
    
    fragColor = vec4((a_0*orig + a_1*sample1 + a_2*sample2 + a_3*sample3 + a_4*sample4).xyz, 1.0);
}

// textureLod(image, geom_texCoord + d * i * dir, level).rgb;