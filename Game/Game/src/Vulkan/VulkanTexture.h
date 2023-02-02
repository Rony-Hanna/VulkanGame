#pragma once

#include "VulkanUtilities.h"
#include <string>
#include <map>

class VulkanTexture
{
public:
	uint32_t CreateTextureImage(const std::string& _fileName);
	CustomImage GetTextureData() const { return m_Texture; }
	static std::map<const uint32_t, const CustomImage> GetTextureDatabase() { return s_TextureDatabase; };

private:
	CustomImage m_Texture;
	static uint32_t s_TextureInstanceCount;
	static std::map<const uint32_t, const CustomImage> s_TextureDatabase;
};
