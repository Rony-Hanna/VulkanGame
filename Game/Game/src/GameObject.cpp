#include "GameObject.h"
#include "Vulkan/VulkanUtilities.h"
#include <glm/gtc/matrix_transform.hpp>

GameObject::GameObject(const VulkanPrimative::Primative _primative, const std::string& _textureName) :
	SceneObject(_primative),
	m_Position(glm::vec3(0.0f)),
	m_Rotation(glm::vec3(0.0f)),
	m_Scale(glm::vec3(1.0f)),
	m_ObjectData{}
{
	if (!_textureName.empty())
	{
		m_ObjectData.texId = m_Texture.CreateTextureImage(_textureName);
	}
}

void GameObject::Cleanup()
{
	// TODO: Move to scene object
	VulkanUtilities::DestroyImageView(m_Texture.GetTextureData().imageView);
	VulkanUtilities::DestroyImage(m_Texture.GetTextureData().image, m_Texture.GetTextureData().imageMemory);

	VulkanUtilities::DestroyBuffer(m_VertexBuffer.buffer, m_VertexBuffer.bufferMemory);
	VulkanUtilities::DestroyBuffer(m_IndexBuffer.buffer, m_IndexBuffer.bufferMemory);
}

void GameObject::Bind(const VkCommandBuffer& _commandBuffer) const
{
	if (m_IsEmptyGameObject) return;

	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(_commandBuffer, 0, 1, &m_VertexBuffer.buffer, offsets);
	vkCmdBindIndexBuffer(_commandBuffer, m_IndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void GameObject::Render(const VkCommandBuffer& _commandBuffer) const
{
	if (m_IsEmptyGameObject) return;

	vkCmdDrawIndexed(_commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
}

void GameObject::UpdateModelMatrix()
{
	m_ObjectData.model = glm::translate(glm::mat4(1.0f), m_Position) *
						 glm::rotate(glm::mat4(1.0f), m_Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
						 glm::rotate(glm::mat4(1.0f), m_Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
						 glm::rotate(glm::mat4(1.0f), m_Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
						 glm::scale(glm::mat4(1.0f), m_Scale);
}
