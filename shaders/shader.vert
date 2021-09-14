#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 frag_color;

layout(push_constant) uniform PushConstantData {
    mat4 transformation;
} push_constant;

void main() {
    gl_Position = push_constant.transformation * vec4(in_position, 1.0);
    frag_color = in_color;
}
