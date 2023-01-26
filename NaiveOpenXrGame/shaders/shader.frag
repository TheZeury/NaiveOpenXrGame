#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec2 FragUv;

layout (binding = 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(textureSampler, FragUv);
    //outColor = vec4(FragUv, 0.0, 1.0);
}
