#version 450

// Ins
layout (location = 0) in vec3 vertex_position;

// Outs
layout (location = 0) out vec4 fragment_color;

// Sampler & textures
layout (set = 1, binding = 0) uniform samplerCube cubemapSampler;

void main()
{
	fragment_color = texture(cubemapSampler, vertex_position);
}