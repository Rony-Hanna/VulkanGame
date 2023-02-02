#version 450

// Ins
layout (location = 0) in vec3 vertex_position;

// Outs
layout (location = 0) out vec4 fragment_color;

// Sampler & textures
layout (set = 1, binding = 0) uniform sampler textureSampler;
layout (set = 2, binding = 0) uniform texture2D textures[2];

void main()
{
	fragment_color = vec4(1.0f, 0.0f, 0.0f, 1.0f); //texture(sampler2D(textures[0], textureSampler), vertex_position);
}