#pragma once

#include <vector>
#include "../Vertex.h"

class VulkanPrimative
{
public:
	enum class Primative { Empty, Triangle, Quad, Cube };
	static void MakePrimative(const Primative _primative, std::vector<Vertex>& _vertices, std::vector<uint32_t>& _indices);

private:
	static void MakeTriangle(std::vector<Vertex>& _vertices, std::vector<uint32_t>& _indices);
	static void MakeQuad(std::vector<Vertex>& _vertices, std::vector<uint32_t>& _indices);
	static void MakeCube(std::vector<Vertex>& _vertices, std::vector<uint32_t>& _indices);
};