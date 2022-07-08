#version 330

uniform sampler2D image0;
uniform sampler2D image1;
uniform sampler2D image2;
uniform sampler2D depth;
uniform sampler2D lightDepth;

uniform vec3 lightPos;
uniform vec3 power;

//uniform mat4 mP;
uniform mat4 mPi;
uniform mat4 mV;
uniform mat4 mVi;
uniform mat4 mVLight;
uniform mat4 mPLight;
//uniform mat4 mM;

in vec2 geom_texCoord;

out vec4 fragColor;

// This is a shader code fragment (not a complete shader) that contains 
// the functions to evaluate the microfacet BRDF.

const float PI = 3.14159265358979323846264;

// The Fresnel reflection factor
//   i -- incoming direction
//   m -- microsurface normal
//   eta -- refractive index
float fresnel(vec3 i, vec3 m, float eta) {
    float c = abs(dot(i, m));
    float g = sqrt(eta * eta - 1.0 + c * c);

    float gmc = g - c;
    float gpc = g + c;
    float nom = c * (g + c) - 1.0;
    float denom = c * (g - c) + 1.0;
    return 0.5 * gmc * gmc / gpc / gpc * (1.0 + nom * nom / denom / denom);
}

// The one-sided Smith shadowing/masking function
//   v -- in or out vector
//   m -- microsurface normal
//   n -- (macro) surface normal
//   alpha -- surface roughness
float G1(vec3 v, vec3 m, vec3 n, float alpha) {
    float vm = dot(v, m);
    float vn = dot(v, n);
    if(vm * vn > 0.0) {
        float cosThetaV = dot(n, v);
        float sinThetaV2 = 1.0 - cosThetaV * cosThetaV;
        float tanThetaV2 = sinThetaV2 / cosThetaV / cosThetaV;
        return 2.0 / (1.0 + sqrt(1.0 + alpha * alpha * tanThetaV2));
    } else {
        return 0;
    }
}

// The GGX slope distribution function
//   m -- microsurface normal
//   n -- (macro) surface normal
//   alpha -- surface roughness
float D(vec3 m, vec3 n, float alpha) {
    float mn = dot(m, n);
    if(mn > 0.0) {
        float cosThetaM = mn;
        float cosThetaM2 = cosThetaM * cosThetaM;
        float tanThetaM2 = (1.0 - cosThetaM2) / cosThetaM2;
        float cosThetaM4 = cosThetaM * cosThetaM * cosThetaM * cosThetaM;
        float X = (alpha * alpha + tanThetaM2);
        return alpha * alpha / (PI * cosThetaM4 * X * X);
    } else {
        return 0.0;
    }
}

// Evalutate the Microfacet BRDF (GGX variant) for the paramters:
//   i -- incoming direction (unit vector, pointing away from surface)
//   o -- outgoing direction (unit vector, pointing away from surface)
//   n -- outward pointing surface normal vector
//   eta -- refractive index
//   alpha -- surface roughness
// return: scalar BRDF value
float isotropicMicrofacet(vec3 i, vec3 o, vec3 n, float eta, float alpha) {

    float odotn = dot(o, n);
    vec3 m = normalize(i + o);

    float idotn = dot(i, n);
    if(idotn <= 0.0 || odotn <= 0.0)
        return 0;

    float idotm = dot(i, m);
    float F = (idotm > 0.0) ? fresnel(i, m, eta) : 0.0;
    float G = G1(i, m, n, alpha) * G1(o, m, n, alpha);
    return F * G * D(m, n, alpha) / (4.0 * idotn * odotn);
}

void main() {

    vec4 data0 = texture(image0, geom_texCoord);
    vec4 data1 = texture(image1, geom_texCoord);
    vec4 data2 = texture(image2, geom_texCoord);

    vec3 k_d = data0.xyz;
    vec3 normal = normalize(data2.xyz * 2.0 - 1.0);

    float alpha = data1.x;
    float eta = data1.y * 100;

    float d = texture(depth, geom_texCoord).x;

    if(d == 1) {
        fragColor = vec4(0);
    } else {

        vec4 ndc = vec4(2.0 * (geom_texCoord.x - 0.5), 2.0 * (geom_texCoord.y - 0.5), 2.0 * (d - 0.5), 1.0);

        vec4 view = mPi * ndc;
        vec4 eyeSpacePos = (view / view.w);

        vec3 worldSpacePos = (mVi * eyeSpacePos).xyz;
        vec4 lightNDC = mPLight * (mVLight * vec4(worldSpacePos, 1.0));
        lightNDC = lightNDC / lightNDC.w;
        vec4 lightTexCords = ((lightNDC + 1.0) / 2.0);
        float distToLight = lightTexCords.z;
        vec2 cords = lightTexCords.xy;
        distToLight = (distToLight - 0.8) * 5.0;
        float shadowMapDepth = texture(lightDepth, cords).x;
        shadowMapDepth = (shadowMapDepth - 0.8) * 5.0;

        float bias = 0.01;
        if(distToLight < shadowMapDepth + bias) {

            vec3 i = -eyeSpacePos.xyz;
            vec3 o = (mV * vec4(lightPos, 1)).xyz + i;
            float r = length(o);
            vec3 normo = normalize(o);
            vec3 normi = normalize(i);
            float micro = isotropicMicrofacet(normi, normo, normal, eta, alpha);
            float NdotH = max(dot(normalize(normal), normo), 0.0);

            vec3 col = ((k_d / PI) + (micro)) * (NdotH * power) / (4 * PI * r * r);
            //fragColor = vec4(shadowMapDepth,shadowMapDepth,shadowMapDepth, 1.0);
            fragColor = vec4(col, 1.0);
        } else {
            fragColor = vec4(0);
        }

    }

}
