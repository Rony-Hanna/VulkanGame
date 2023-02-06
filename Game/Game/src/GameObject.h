#pragma once

#include "SceneObject.h"
#include "Vulkan/VulkanTexture.h"

struct ObjectData
{
	glm::mat4 model;
	uint32_t texId;
};

class GameObject : public SceneObject
{
public:
	GameObject(const VulkanPrimative::Primative _primative = VulkanPrimative::Primative::Empty, const std::string& _textureName = "");

	void CleanUp();
	void UpdateModelMatrix();
	void SetPosition(const glm::vec3& _pos) { m_Position = _pos; };
	void SetScale(const glm::vec3& _scale) { m_Scale = _scale; };
	void SetRotation(const glm::vec3& _rotation) { m_Rotation = _rotation; };
	uint32_t GetTexId() const { return m_ObjectData.texId; };
	ObjectData GetObjectData() const { return m_ObjectData; };
	CustomImage GetTextureData() const { return m_Texture.GetTextureData(); };

private:
	glm::vec3 m_Position;
	glm::vec3 m_Rotation;
	glm::vec3 m_Scale;
	ObjectData m_ObjectData;
	VulkanTexture m_Texture;
};
