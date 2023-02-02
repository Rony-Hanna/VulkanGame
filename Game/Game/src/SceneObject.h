#pragma once

#include "Vertex.h"
#include "Vulkan/VulkanPrimative.h"
#include "Vulkan/VulkanUtilities.h"

struct MainDevice;

class SceneObject
{
public:
	SceneObject(const VulkanPrimative::Primative _primative);

protected:
	UniformBuffer m_VertexBuffer;
	UniformBuffer m_IndexBuffer;
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	bool m_IsEmptyGameObject;

private:
	void CreateVertexBuffer();
	void CreateIndexBuffer();
};
