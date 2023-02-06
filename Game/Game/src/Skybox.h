#pragma once

#include "SceneObject.h"
#include "Vulkan/VulkanTexture.h"

class Skybox : public SceneObject
{
public:
	void LoadSkyboxTextures(const std::vector<std::string>& _skyboxTextures);
	void CleanUp();
	VulkanTexture GetCubemapTexture() const { return m_Texture; };

private:
	VulkanTexture m_Texture;
};
