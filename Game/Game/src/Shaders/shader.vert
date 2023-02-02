#version 450

// Ins
layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_color;
layout (location = 2) in vec2 vertex_uv;

// Outs
layout (location = 0) out vec3 vertex_outColor;
layout (location = 1) out vec2 vertex_outUV;
layout (location = 2) out flat int outTexId;

// Uniform buffers
layout (set = 0, binding = 0) uniform UBO
{
	mat4 view;
	mat4 proj;
} u_ViewProj;

// Push constants
layout (push_constant) uniform pushConstants
{
	mat4 model;
	int texId;
} u_PushConstants;

void main()
{
	gl_Position = u_ViewProj.proj * u_ViewProj.view * u_PushConstants.model * vec4(vertex_position, 1.0f);
	vertex_outColor = vertex_color;
	vertex_outUV = vertex_uv;
	outTexId = u_PushConstants.texId;
}