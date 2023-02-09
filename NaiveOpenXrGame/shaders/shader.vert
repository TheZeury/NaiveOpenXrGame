#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

//layout (location = 0) out vec3 fragColor;
layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec2 fragUv;

layout (push_constant) uniform Push {
    mat4 projectionView;
    mat4 modelMatrix;
} push;

void main() {
    gl_Position = push.projectionView * vec4(position, 1.0);
    //fragColor = color.xyz;
    fragPosition = (push.modelMatrix * vec4(position, 1.0)).xyz;
    fragNormal = (push.modelMatrix * vec4(normal, 0.0)).xyz;
    fragUv = uv;
}
