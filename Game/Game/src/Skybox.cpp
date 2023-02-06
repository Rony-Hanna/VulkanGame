#include "Skybox.h"

void Skybox::LoadSkyboxTextures(const std::vector<std::string>& _skyboxTextures)
{
	SceneObject::Init(VulkanPrimative::Primative::Cube);

	m_Texture.CreateCubemapTexture(_skyboxTextures);
}

void Skybox::CleanUp()
{
	VulkanUtilities::DestroyImageView(m_Texture.GetTextureData().imageView);
	VulkanUtilities::DestroyImage(m_Texture.GetTextureData().image, m_Texture.GetTextureData().imageMemory);

	SceneObject::CleanUp();
}
