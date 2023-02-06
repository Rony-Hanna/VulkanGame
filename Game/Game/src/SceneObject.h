#pragma once

#include "Vertex.h"
#include "Vulkan/VulkanPrimative.h"
#include "Vulkan/VulkanUtilities.h"

struct MainDevice;

class SceneObject
{
public:
	SceneObject();
	SceneObject(const VulkanPrimative::Primative _primative);

	void Init(const VulkanPrimative::Primative _primative);
	void Bind(const VkCommandBuffer& _commandBuffer) const;
	void Render(const VkCommandBuffer& _commandBuffer) const;
	void CleanUp();

private:
	void CreateVertexBuffer();
	void CreateIndexBuffer();

private:
	UniformBuffer m_VertexBuffer;
	UniformBuffer m_IndexBuffer;
	std::vector<Vertex> m_Vertices;
	std::vector<uint32_t> m_Indices;
	bool m_IsEmptyGameObject;
};
