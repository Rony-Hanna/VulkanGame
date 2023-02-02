#version 450

// Ins
layout (location = 0) in vec3 vertex_color;
layout (location = 1) in vec2 vertex_uv;
layout (location = 2) in flat int texId;

// Outs
layout (location = 0) out vec4 fragment_color;

// Sampler & textures
layout (set = 1, binding = 0) uniform sampler textureSampler;
layout (set = 2, binding = 0) uniform texture2D textures[2];

void main()
{
	fragment_color = texture(sampler2D(textures[texId], textureSampler), vertex_uv);
}