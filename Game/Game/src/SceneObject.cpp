#include "SceneObject.h"
#include "Vulkan/VulkanUtilities.h"

SceneObject::SceneObject() : 
	m_VertexBuffer{},
	m_IndexBuffer{},
	m_Vertices{},
	m_Indices{},
	m_IsEmptyGameObject(true)
{}

SceneObject::SceneObject(const VulkanPrimative::Primative _primative) :
	m_VertexBuffer{},
	m_IndexBuffer{},
	m_Vertices{},
	m_Indices{},
	m_IsEmptyGameObject(false)
{
	Init(_primative);
}

void SceneObject::Init(const VulkanPrimative::Primative _primative)
{
	if (_primative != VulkanPrimative::Primative::Empty)
	{
		VulkanPrimative::MakePrimative(_primative, m_Vertices, m_Indices);
		CreateVertexBuffer();
		CreateIndexBuffer();
		m_IsEmptyGameObject = false;
	}
	else
	{
		m_IsEmptyGameObject = true;
	}
}

void SceneObject::Bind(const VkCommandBuffer& _commandBuffer) const
{
	if (m_IsEmptyGameObject) return;

	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, &m_VertexBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(_commandBuffer, m_IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void SceneObject::Render(const VkCommandBuffer& _commandBuffer) const
{
	if (m_IsEmptyGameObject) return;

	vkCmdDrawIndexed(_commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
}

void SceneObject::CleanUp()
{
	VulkanUtilities::DestroyBuffer(m_VertexBuffer.buffer, m_VertexBuffer.bufferMemory);
	VulkanUtilities::DestroyBuffer(m_IndexBuffer.buffer, m_IndexBuffer.bufferMemory);
}

void SceneObject::CreateVertexBuffer()
{
	VkDeviceSize sizeOfBuffer = sizeof(Vertex) * m_Vertices.size();
	VkBuffer stagingBuffer{};
	VkDeviceMemory stagingBufferMemory{};

	BufferInfo stagingBufferInfo{};
	stagingBufferInfo.pBuffer = &stagingBuffer;
	stagingBufferInfo.pBufferMemory = &stagingBufferMemory;
	stagingBufferInfo.bufferSize = sizeOfBuffer;
	stagingBufferInfo.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	VulkanUtilities::CreateBuffer(stagingBufferInfo);

	void* stagingBufferData = nullptr;
	VulkanUtilities::MapMemory(stagingBufferMemory, sizeOfBuffer, &stagingBufferData);
	memcpy(stagingBufferData, m_Vertices.data(), static_cast<size_t>(sizeOfBuffer));
	VulkanUtilities::UnmapMemory(stagingBufferMemory);

	BufferInfo vertexBufferInfo{};
	vertexBufferInfo.pBuffer = &m_VertexBuffer.buffer;
	vertexBufferInfo.pBufferMemory = &m_VertexBuffer.bufferMemory;
	vertexBufferInfo.bufferSize = sizeOfBuffer;
	vertexBufferInfo.bufferUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	vertexBufferInfo.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VulkanUtilities::CreateBuffer(vertexBufferInfo);

	VulkanUtilities::CopyBuffer(stagingBuffer, m_VertexBuffer.buffer, sizeOfBuffer);

	VulkanUtilities::DestroyBuffer(stagingBuffer, stagingBufferMemory);
}

void SceneObject::CreateIndexBuffer()
{
	VkDeviceSize sizeOfBuffer = sizeof(uint32_t) * m_Indices.size();
	VkBuffer stagingBuffer{};
	VkDeviceMemory stagingBufferMemory{};

	BufferInfo stagingBufferInfo{};
	stagingBufferInfo.pBuffer = &stagingBuffer;
	stagingBufferInfo.pBufferMemory = &stagingBufferMemory;
	stagingBufferInfo.bufferSize = sizeOfBuffer;
	stagingBufferInfo.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferInfo.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	VulkanUtilities::CreateBuffer(stagingBufferInfo);

	void* stagingBufferData = nullptr;
	VulkanUtilities::MapMemory(stagingBufferMemory, sizeOfBuffer, &stagingBufferData);
	memcpy(stagingBufferData, m_Indices.data(), static_cast<size_t>(sizeOfBuffer));
	VulkanUtilities::UnmapMemory(stagingBufferMemory);

	BufferInfo indicesBufferInfo{};
	indicesBufferInfo.pBuffer = &m_IndexBuffer.buffer;
	indicesBufferInfo.pBufferMemory = &m_IndexBuffer.bufferMemory;
	indicesBufferInfo.bufferSize = sizeOfBuffer;
	indicesBufferInfo.bufferUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	indicesBufferInfo.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VulkanUtilities::CreateBuffer(indicesBufferInfo);

	VulkanUtilities::CopyBuffer(stagingBuffer, m_IndexBuffer.buffer, sizeOfBuffer);

	VulkanUtilities::DestroyBuffer(stagingBuffer, stagingBufferMemory);
}
