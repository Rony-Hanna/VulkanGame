#version 450

// Ins
layout (location = 0) in vec3 vertex_position;

// Outs
layout (location = 0) out vec3 vertex_outPosition;

// Uniform buffers
layout (set = 0, binding = 0) uniform ubo
{
	mat4 view;
	mat4 proj;
} u_ViewProj;

void main()
{
	gl_Position = u_ViewProj.proj * mat4(mat3(u_ViewProj.view)) * vec4(vertex_position, 1.0f);
	vertex_outPosition = vertex_position;
	vertex_outPosition.y = -vertex_outPosition.y; // TEMP: May be bad but this is to fix the cubemap being inverted
}