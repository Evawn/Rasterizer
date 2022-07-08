#version 330

uniform sampler2D diffuse;
uniform sampler2D normal;
uniform vec3 radiance;
uniform sampler2D depth;
uniform float range;

uniform mat4 mP;
uniform mat4 mV;
// uniform mat4 mVi;

vec3 sunskyRadiance(vec3 dir);

in vec2 geom_texCoord;

out vec4 fragColor;

const float PI = 3.14159265358979323846264;

float random(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 squareToUniformDiskConcentric(vec2 sample) {
    float r1 = (2.0 * sample.x) - 1.0;
    float r2 = (2.0 * sample.y) - 1.0;

    float phi, r;
    if(r1 == 0 && r2 == 0) {
        r = phi = 0;
    } else if(r1 * r1 > r2 * r2) {
        r = r1;
        phi = (PI / 4.0) * (r2 / r1);
    } else {
        r = r2;
        phi = (PI / 2.0) - (r1 / r2) * (PI / 4.0);
    }

    if(r < 0) {
        r = -sqrt(-r);
    } else {
        r = sqrt(r);
    }

    float cosPhi = cos(phi);
    float sinPhi = sin(phi);

    return vec2(r * cosPhi, r * sinPhi);
}

vec3 squareToCosineHemisphere(vec2 sample) {
    vec2 p = squareToUniformDiskConcentric(sample);
    float z = sqrt(1.0 - (p.x * p.x) - (p.y * p.y));

    /* Guard against numerical imprecisions */
    if(z == 0) {
        z = 1e-10;
    }

    return vec3(p.x, p.y, z);
}

void main() {
    float d = texture(depth, geom_texCoord).x;

    if(d == 1) {
        vec4 farNDC = vec4(2.0 * (geom_texCoord.x - 0.5), 2.0 * (geom_texCoord.y - 0.5), 1, 1.0);
        vec4 farView = inverse(mP) * farNDC;
        vec4 farEye = (farView / farView.w);
        vec4 farWorld = inverse(mV) * farEye;

        vec4 nearNDC = vec4(2.0 * (geom_texCoord.x - 0.5), 2.0 * (geom_texCoord.y - 0.5), -1, 1.0);
        vec4 nearView = inverse(mP) * nearNDC;
        vec4 nearEye = (nearView / nearView.w);
        vec4 nearWorld = inverse(mV) * nearEye;

        vec3 dir = normalize((farWorld - nearWorld).xyz);
        fragColor = vec4(sunskyRadiance(dir), 1.0);
    } else {
        vec3 diffuseColor = texture(diffuse, geom_texCoord).xyz;
        vec3 norm = (texture(normal, geom_texCoord).xyz * 2.0) - 1.0;
        norm = normalize(norm);

        vec3 tang1;
        if(norm.x < norm.y && norm.x < norm.z) {
            tang1 = vec3(1, 0, 0);
        } else if(norm.y < norm.x && norm.y < norm.z) {
            tang1 = vec3(0, 1, 0);
        } else if(norm.z < norm.y && norm.z < norm.x) {
            tang1 = vec3(0, 0, 1);
        } else {
            tang1 = vec3(1, 0, 0);
        }

        vec3 tang2 = normalize(cross(norm, tang1));
        tang1 = normalize(cross(tang2, norm));

        vec4 ndc = vec4(2.0 * (geom_texCoord.x - 0.5), 2.0 * (geom_texCoord.y - 0.5), 2.0 * (d - 0.5), 1.0);
        vec4 view = inverse(mP) * ndc;
        vec4 eyeSpacePos = (view / view.w);
        // vec4 worldSpacePos = inverse(mV) * eyeSpacePos;

        mat4 basis = mat4(vec4(tang1, 0), vec4(tang2, 0), vec4(norm, 0), eyeSpacePos);

        vec2 randSquare = vec2(random(geom_texCoord), random(geom_texCoord + 2.3));
        float r = sqrt(random(geom_texCoord + vec2(2) + 2 * 2)); //r^2 distributed

        vec3 randHem = r * squareToCosineHemisphere(randSquare);
        vec4 sampleSurf = vec4(randHem * range, 1);
        // vec4 sampleSurf = vec4(vec3(0, 0, 0.1) * range, 1);
        vec4 sampleEye = basis * sampleSurf;

        vec4 sampleNDC = mP * sampleEye;
        vec3 sampleScreen = ((sampleNDC / sampleNDC.w).xyz + 1) / 2;
        float sampleDepth = sampleScreen.z;

        float depthToCheck = texture(depth, sampleScreen.xy).x;
        float bias = 0.00001;
        float rangeCheck = abs(sampleDepth - d) > 4 * range ? 0.0 : 1.0; // IDK IF THIS IS NEEDED

        if(sampleDepth < depthToCheck + bias) {
            fragColor = rangeCheck * vec4(radiance * diffuseColor, 1.0);
        } else {
            fragColor = vec4(0, 0, 0, 1);
        }
    }
}