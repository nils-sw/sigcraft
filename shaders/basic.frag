#version 450
#extension GL_EXT_mesh_shader : require
#extension GL_EXT_shader_image_load_formatted : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_buffer_reference : require


layout(location = 1) perprimitiveEXT in PrimitiveInput{
    vec4 color;
} primitiveInput;

// layout(location = 1)
// in vec3 normal;

layout(location = 0)
out vec4 colorOut;

void main() {
    colorOut = primitiveInput.color;
}