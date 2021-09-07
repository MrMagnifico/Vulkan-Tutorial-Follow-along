#version 450

layout(location = 0) in vec3 frag_colour;

layout(location = 0) out vec4 out_colour;

layout(push_constant) uniform PushConstantData {
    mat4 transformation;
} push_constant;

void main() {
    out_colour = vec4(frag_colour, 1.0);
}
