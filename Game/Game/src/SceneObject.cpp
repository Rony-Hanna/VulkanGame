#include "SceneObject.h"
#include "Vulkan/VulkanUtilities.h"

SceneObject::SceneObject(const VulkanPrimative::Primative _primative) :
	m_VertexBuffer{},
	m_IndexBuffer{},
	m_Vertices{},
	m_Indices{},
	m_IsEmptyGameObject(false)
{
	if (_primative != VulkanPrimative::Primative::Empty)
	{
		VulkanPrimative::MakePrimative(_primative, m_Vertices, m_Indices);
		CreateVertexBuffer();
		CreateIndexBuffer();
	}
	else
	{
		m_IsEmptyGameObject = true;
	}
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
