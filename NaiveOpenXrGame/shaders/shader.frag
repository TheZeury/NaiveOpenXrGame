#version 450

//layout (location = 0) in vec3 fragColor; 
layout (location = 0) in vec3 fragPosition;
layout (location = 1) in vec3 fragNormal;
layout (location = 2) in vec2 FragUv;

layout (binding = 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor;

vec3 lightDirection = { 1.0, 1.0, -0.4 };

vec4 lightColor = { 0.5, 0.5, 0.5, 1.0 };
float ambient = 0.5;
float diffuse;

void main() {
    vec4 ambientLight = lightColor * ambient;
    diffuse = max(dot(lightDirection, normalize(fragNormal)), 0.0);
    vec4 diffuseLight = lightColor * diffuse;
    outColor = texture(textureSampler, FragUv) * (ambientLight + diffuseLight);
    if(outColor.w < 0.3)
    {
        discard;
    }
    //outColor = vec4(FragUv, 0.0, 1.0);
}
