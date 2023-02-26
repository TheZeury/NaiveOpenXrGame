#version 450

//layout (location = 0) in vec3 fragColor; 
layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec2 FragUv;
layout (location = 3) in mat3 TBN;

layout (binding = 0) uniform MaterialPropertyBuffer {
    bool enableDiffuseMap;
    bool enableNormalMap;
    vec4 defaultColor;
} materialProperties;
layout (binding = 1) uniform sampler2D diffuseSampler;
layout (binding = 2) uniform sampler2D normalSampler;

layout (location = 0) out vec4 outColor;

vec3 lightDirection = { 1.0, 1.0, -0.4 };

vec4 lightColor = { 1.0, 1.0, 1.0, 1.0 };
float ambientValue = 0.4;
float diffuseValue;

void main() {
    vec3 normal = normalize(materialProperties.enableNormalMap ? (TBN * (texture(normalSampler, FragUv).rgb * 2.0 - 1.0)) : fragNormal);
    vec4 ambientLight = lightColor * ambientValue;
    diffuseValue = max(dot(lightDirection, normalize(normal)), 0.0);
    vec4 diffuseLight = lightColor * diffuseValue;
    outColor = (materialProperties.enableDiffuseMap ? texture(diffuseSampler, FragUv) : materialProperties.defaultColor) * (ambientLight + diffuseLight);
    if(outColor.w < 0.3)
    {
        discard;
    }
    //outColor = vec4(normal, 1.0);
    //outColor = vec4(0.5 * (normalize(normal) + 1.0), 1.0);
    //if(outColor == vec4(0.0, 0.0, 0.0, 1.0)) outColor = vec4(1.0, 1.0, 1.0, 1.0);
    //outColor = vec4(FragUv, 0.0, 1.0);
}
