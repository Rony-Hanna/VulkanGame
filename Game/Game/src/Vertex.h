#pragma once

#include <glm/glm.hpp>

struct Vertex
{
public:
	Vertex(const glm::vec3& _pos = glm::vec3(0.0f), 
		   const glm::vec3& _col = glm::vec3(1.0f),
		   const glm::vec2& _uv = glm::vec2(1.0f)) :
		position(_pos),
		color(_col),
		uv(_uv)
	{}

	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 uv;
};