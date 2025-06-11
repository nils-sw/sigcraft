#version 450
#extension GL_EXT_shader_image_load_formatted : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference : require

layout(location = 0)
out vec3 color;

layout(location = 0)
in vec3 vertex;

layout(scalar, push_constant) uniform T {
    mat4 matrix;
    ivec3 chunk_position;
    float time;
} push_constants;

void main() {
    mat4 matrix = push_constants.matrix;
    gl_Position = matrix * vec4(vertex + push_constants.chunk_position * 16, 1.0);
    int primid = gl_VertexIndex / 6;
    color = vec3(primid % 2, (primid % 4) / 4.0, (primid % 3) / 3.0);
}