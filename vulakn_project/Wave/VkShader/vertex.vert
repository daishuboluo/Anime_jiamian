#version 450

layout(location = 0) in vec2 inPos;

layout(push_constant) uniform PushConstants {
    mat4 projection;
} pc;

layout(location = 0) out float vLineCoord;

void main() {
    vLineCoord = float(gl_VertexIndex % 2);
    gl_Position = pc.projection * vec4(inPos, 0.0, 1.0);
}