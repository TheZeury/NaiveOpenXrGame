#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out vec2 fragUv;

layout (push_constant) uniform Push {
    mat4 projectionView;
} push;

void main() {
    gl_Position = push.projectionView * vec4(position, 1.0);
    fragColor = color.xyz;
    fragUv = uv;
}
