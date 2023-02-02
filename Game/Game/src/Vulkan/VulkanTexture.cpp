#include "VulkanTexture.h"
#include "VulkanInit.h"
#include "../Utilities.h"
#include <stdexcept>

uint32_t VulkanTexture::s_TextureInstanceCount = 0;
std::map<const uint32_t, const CustomImage> VulkanTexture::s_TextureDatabase{};

uint32_t VulkanTexture::CreateTextureImage(const std::string& _fileName)
{
	for (auto iter = s_TextureDatabase.begin(); iter != s_TextureDatabase.end(); ++iter)
	{
		if (iter->second.imageTitle == _fileName)
		{
			return iter->second.imageId;
		}
	}

	int width = 0;
	int height = 0;
	int desiredChannels = 0;

	unsigned char* imageData = Utilities::LoadTextureFile(_fileName, &width, &height, &desiredChannels);

	VkBuffer imageBuffer{};
	VkDeviceMemory imageBufferMemory{};
	VkDeviceSize imageBufferSize = width * height * desiredChannels;

	BufferInfo imageStagingBuffer{};
	imageStagingBuffer.bufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	imageStagingBuffer.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	imageStagingBuffer.bufferSize = imageBufferSize;
	imageStagingBuffer.pBufferMemory = &imageBufferMemory;
	imageStagingBuffer.pBuffer = &imageBuffer;
	VulkanUtilities::CreateBuffer(imageStagingBuffer);

	// Copy image data to staging buffer
	void* data = nullptr;
	VulkanUtilities::MapMemory(imageBufferMemory, imageBufferSize, &data);
	memcpy(data, imageData, static_cast<size_t>(imageBufferSize));
	VulkanUtilities::UnmapMemory(imageBufferMemory);

	Utilities::FreeImage(imageData);

	// Create texture image
	VkExtent2D imageDimensions{};
	imageDimensions.width = static_cast<uint32_t>(width);
	imageDimensions.height = static_cast<uint32_t>(height);

	m_Texture = VulkanUtilities::CreateImage(imageDimensions, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// Transition layout of the image to be compatible for copy operation
	VulkanUtilities::TransitionImageLayout(m_Texture.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	// Copy image buffer data to image
	VulkanUtilities::CopyBufferToImage(*imageStagingBuffer.pBuffer, m_Texture.image, { static_cast<uint32_t>(width), static_cast<uint32_t>(height) });

	// Transition layout of the image to be compatible for shader reading
	VulkanUtilities::TransitionImageLayout(m_Texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	VulkanUtilities::DestroyBuffer(*imageStagingBuffer.pBuffer, *imageStagingBuffer.pBufferMemory);

	// Create image view for the texture
	VkImageViewCreateInfo imageViewCreateInfo = Vki::ImageViewCreateInfo(m_Texture.image, m_Texture.imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

	VkResult re = vkCreateImageView(*VulkanUtilities::GetDevice(), &imageViewCreateInfo, nullptr, &m_Texture.imageView);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create an image view\n");

	uint32_t textureId = s_TextureInstanceCount;
	m_Texture.imageId = textureId;
	m_Texture.imageTitle = _fileName;

	s_TextureDatabase.insert(std::pair(textureId++, m_Texture));
	return s_TextureInstanceCount++;
}
