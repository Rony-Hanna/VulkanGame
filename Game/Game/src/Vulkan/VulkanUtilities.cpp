#include "VulkanUtilities.h"
#include "VulkanDevice.h"
#include "VulkanInit.h"
#include "VulkanSwapchain.h"
#include <stdexcept>

const MainDevice* VulkanUtilities::m_MainDevice = nullptr;

void VulkanUtilities::Initialize(const MainDevice* _mainDevice)
{
	m_MainDevice = _mainDevice;
}

void VulkanUtilities::CheckRequiredInstanceExtensions(const std::vector<const char*>& _requiredInstanceExtensions)
{
	uint32_t numOfAvailableExtensions = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &numOfAvailableExtensions, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(numOfAvailableExtensions);
	vkEnumerateInstanceExtensionProperties(nullptr, &numOfAvailableExtensions, availableExtensions.data());

	for (uint8_t i = 0; i < _requiredInstanceExtensions.size(); ++i)
	{
		bool foundExtension = false;

		for (const auto& extension : availableExtensions)
		{
			if (std::strcmp(_requiredInstanceExtensions[i], extension.extensionName) == 0)
			{
				foundExtension = true;
				break;
			}
		}

		if (!foundExtension)
		{
			throw std::runtime_error("VULKAN ERROR: Failed to retrieve required instance extensions\n");
		}
	}
}

void VulkanUtilities::CheckRequiredLayers(const std::vector<const char*>& _requiredLayers)
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const auto& requiredLayer : _requiredLayers)
	{
		bool foundLayer = false;

		for (uint8_t i = 0; i < availableLayers.size(); ++i)
		{
			if (std::strcmp(requiredLayer, availableLayers[i].layerName) == 0)
			{
				foundLayer = true;
				break;
			}
		}

		if (!foundLayer)
		{
			throw std::runtime_error("VULKAN ERROR: Failed to retrieve required layers\n");
		}
	}
}

uint32_t VulkanUtilities::FindMemoryIndex(const uint32_t _memoryTypeBits, const VkMemoryPropertyFlags& _memoryProperties)
{
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties{};
	vkGetPhysicalDeviceMemoryProperties(m_MainDevice->physicalDevice, &deviceMemoryProperties);

	VkMemoryPropertyFlags requiredMemory = _memoryProperties;

	for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; ++i)
	{
		if ((_memoryTypeBits & (1 << i)) &&
			(deviceMemoryProperties.memoryTypes[i].propertyFlags & requiredMemory) == requiredMemory)
		{
			return i;
		}
	}

	throw std::runtime_error("VULKAN ERROR: Failed to find memory index\n");
}

VkShaderModule VulkanUtilities::CreateShaderModule(const std::vector<char>& _shaderCode)
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo = Vki::ShaderModuleCreateInfo(_shaderCode);

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	VkResult re = vkCreateShaderModule(m_MainDevice->device, &shaderModuleCreateInfo, nullptr, &shaderModule);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create Vulkan Shader Module\n");

	return shaderModule;
}

CustomImage VulkanUtilities::CreateImage(const VkExtent2D& _dimensions, const VkFormat _format, const VkImageUsageFlags _usage, const VkMemoryPropertyFlags _memoryProperty)
{
	CustomImage customImage{};
	customImage.imageFormat = _format;

	VkImageCreateInfo imageCreateInfo = Vki::ImageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;					// Type of image (1D, 2D, or 3D)
	imageCreateInfo.extent.width = _dimensions.width;				// Width of image extent
	imageCreateInfo.extent.height = _dimensions.height;				// Height of image extent
	imageCreateInfo.extent.depth = 1;								// Depth of image (just 1, no 3D aspect)
	imageCreateInfo.mipLevels = 1;									// Number of mipmap levels
	imageCreateInfo.arrayLayers = 1;								// Number of levels in image array
	imageCreateInfo.format = _format;								// Format type of the image
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		// Layout of image data on creation
	imageCreateInfo.usage = _usage;									// Bit flags defining what image will be used for
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;				// How image data should be "tiled" (arranged for optimal reading)
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;				// Number of samples for multisampling		
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		// Whether image can be shared between queues

	VkResult re = vkCreateImage(m_MainDevice->device, &imageCreateInfo, nullptr, &customImage.image);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create image\n");

	VkMemoryRequirements memoryRequirements{};
	vkGetImageMemoryRequirements(m_MainDevice->device, customImage.image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocInfo{};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memoryRequirements.size;
	memoryAllocInfo.memoryTypeIndex = VulkanUtilities::FindMemoryIndex(memoryRequirements.memoryTypeBits, _memoryProperty);

	re = vkAllocateMemory(m_MainDevice->device, &memoryAllocInfo, nullptr, &customImage.imageMemory);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to allocate memory for image\n");

	// Connect memory to image
	vkBindImageMemory(m_MainDevice->device, customImage.image, customImage.imageMemory, 0);

	return customImage;
}

CustomImage VulkanUtilities::CreateCubemapImage(const VkExtent2D& _dimensions, const VkFormat _format, const VkImageUsageFlags _usage, const VkMemoryPropertyFlags _memoryProperty)
{
	CustomImage layeredImage{};
	layeredImage.imageFormat = _format;

	VkImageCreateInfo imageCreateInfo = Vki::ImageCreateInfo();
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;					
	imageCreateInfo.extent.width = _dimensions.width;				
	imageCreateInfo.extent.height = _dimensions.height;				
	imageCreateInfo.extent.depth = 1;								
	imageCreateInfo.mipLevels = 1;									
	imageCreateInfo.arrayLayers = 6;								
	imageCreateInfo.format = _format;								
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;		
	imageCreateInfo.usage = _usage;									
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;				
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;				
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;		

	VkResult re = vkCreateImage(m_MainDevice->device, &imageCreateInfo, nullptr, &layeredImage.image);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create image\n");

	VkMemoryRequirements memoryRequirements{};
	vkGetImageMemoryRequirements(m_MainDevice->device, layeredImage.image, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocInfo{};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memoryRequirements.size;
	memoryAllocInfo.memoryTypeIndex = VulkanUtilities::FindMemoryIndex(memoryRequirements.memoryTypeBits, _memoryProperty);

	re = vkAllocateMemory(m_MainDevice->device, &memoryAllocInfo, nullptr, &layeredImage.imageMemory);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to allocate memory for image\n");

	// Connect memory to image
	vkBindImageMemory(m_MainDevice->device, layeredImage.image, layeredImage.imageMemory, 0);

	return layeredImage;
}

void VulkanUtilities::GetSwapchainInfo(const VkPhysicalDevice& _physicalDevice, const VkSurfaceKHR& _surface, SwapchainInfo& _swapchainInfo)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &_swapchainInfo.surfaceCapabilities);

	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &surfaceFormatCount, nullptr);

	if (surfaceFormatCount != 0)
	{
		_swapchainInfo.surfaceFormats.resize(surfaceFormatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &surfaceFormatCount, _swapchainInfo.surfaceFormats.data());
	}

	uint32_t presentationCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentationCount, nullptr);

	if (presentationCount != 0)
	{
		_swapchainInfo.surfacePresentModes.resize(presentationCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentationCount, _swapchainInfo.surfacePresentModes.data());
	}
}

const VkDevice* VulkanUtilities::GetDevice()
{
	return &m_MainDevice->device;
}

void VulkanUtilities::BeginCommandBuffer(VkCommandBuffer* _commandBuffer)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = *m_MainDevice->queueFamilyIndices.commandPool;
	allocInfo.commandBufferCount = 1;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(m_MainDevice->device, &allocInfo, _commandBuffer);

	VkCommandBufferBeginInfo bufferBeginInfo{};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Only use command buffer once

	vkBeginCommandBuffer(*_commandBuffer, &bufferBeginInfo);
}

void VulkanUtilities::EndCommandBuffer(VkCommandBuffer* _commandBuffer)
{
	vkEndCommandBuffer(*_commandBuffer);
}

void VulkanUtilities::SubmitCommandBuffer(VkCommandBuffer* _commandBuffer)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = _commandBuffer;
	vkQueueSubmit(m_MainDevice->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_MainDevice->graphicsQueue);
}

void VulkanUtilities::CreateBuffer(BufferInfo& _bufferInfo)
{
	VkBufferCreateInfo bufferCreateInfo = Vki::BufferCreateInfo(_bufferInfo.bufferUsage, _bufferInfo.bufferSize);

	VkResult re = vkCreateBuffer(m_MainDevice->device, &bufferCreateInfo, nullptr, _bufferInfo.pBuffer);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create buffer\n");

	VkMemoryRequirements memoryRequirements{};
	vkGetBufferMemoryRequirements(m_MainDevice->device, *_bufferInfo.pBuffer, &memoryRequirements);

	const uint32_t memoryTypeIndex = FindMemoryIndex(memoryRequirements.memoryTypeBits, _bufferInfo.memoryProperties);
	VkMemoryAllocateInfo memoryAllocation = Vki::AllocateMemoryInfo(memoryRequirements.size, memoryTypeIndex);

	re = vkAllocateMemory(m_MainDevice->device, &memoryAllocation, nullptr, _bufferInfo.pBufferMemory);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to allocate buffer memory\n");

	// Connect memory to buffer
	vkBindBufferMemory(m_MainDevice->device, *_bufferInfo.pBuffer, *_bufferInfo.pBufferMemory, 0);
}

void VulkanUtilities::DestroyBuffer(const VkBuffer& _buffer, const VkDeviceMemory& _bufferMemory)
{
	vkDestroyBuffer(m_MainDevice->device, _buffer, nullptr);
	vkFreeMemory(m_MainDevice->device, _bufferMemory, nullptr);
}

void VulkanUtilities::DestroyImage(const VkImage& _image, const VkDeviceMemory& _imageMemory)
{
	vkDestroyImage(m_MainDevice->device, _image, nullptr);
	vkFreeMemory(m_MainDevice->device, _imageMemory, nullptr);
}

void VulkanUtilities::DestroyImageView(const VkImageView& _imageView)
{
	vkDestroyImageView(m_MainDevice->device, _imageView, nullptr);
}

void VulkanUtilities::CopyBuffer(VkBuffer& _srcBuffer, VkBuffer& _dstBuffer, const VkDeviceSize& _bufferSize)
{
	VkCommandBuffer transferCommandBuffer{};
	BeginCommandBuffer(&transferCommandBuffer);

	// Copy buffer - buffer 
	// Configure buffer region to copy to
	VkBufferCopy bufferRegion{};
	bufferRegion.srcOffset = 0;			// Start copying from start of source buffer
	bufferRegion.dstOffset = 0;			// Start pasting to start of destination buffer
	bufferRegion.size = _bufferSize;	// Size of the buffer to copy
	vkCmdCopyBuffer(transferCommandBuffer, _srcBuffer, _dstBuffer, 1, &bufferRegion);

	EndCommandBuffer(&transferCommandBuffer);
	SubmitCommandBuffer(&transferCommandBuffer);

	vkFreeCommandBuffers(m_MainDevice->device, *m_MainDevice->queueFamilyIndices.commandPool, 1, &transferCommandBuffer);
}

void VulkanUtilities::CopyBufferToImage(VkBuffer& _srcBuffer, VkImage& _dstImage, const VkExtent2D& _imageDimensions, uint32_t _layerCount)
{
	VkCommandBuffer transferCommandBuffer{};
	BeginCommandBuffer(&transferCommandBuffer);

	// Copy buffer - image
	// Configure image region to copy to
	VkBufferImageCopy imageRegion{};
	imageRegion.bufferOffset = 0;														// Offset into buffer data
	imageRegion.bufferRowLength = 0;													// Row length of data to calculate data spacing
	imageRegion.bufferImageHeight = 0;													// Image height to calculate data spacing
	imageRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;				// Which aspect of image to copy
	imageRegion.imageSubresource.mipLevel = 0;											// Mipmap level to copy
	imageRegion.imageSubresource.baseArrayLayer = 0;									// Starting array layer (if array)
	imageRegion.imageSubresource.layerCount = _layerCount;										// Number of layers to copy starting at base array later
	imageRegion.imageOffset = { 0, 0, 0 };												// Offset into image (as opposed to raw data in buffer offset)
	imageRegion.imageExtent = { _imageDimensions.width, _imageDimensions.height, 1 };	// Size of region to copy as (x, y, z) values
	vkCmdCopyBufferToImage(transferCommandBuffer, _srcBuffer, _dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageRegion);

	EndCommandBuffer(&transferCommandBuffer);
	SubmitCommandBuffer(&transferCommandBuffer);

	vkFreeCommandBuffers(m_MainDevice->device, *m_MainDevice->queueFamilyIndices.commandPool, 1, &transferCommandBuffer);
}

void VulkanUtilities::MapMemory(const VkDeviceMemory& _memoryToMap, const VkDeviceSize& _sizeOfMemory, void** _pData)
{
	vkMapMemory(m_MainDevice->device, _memoryToMap, 0, _sizeOfMemory, 0, _pData);
}

void VulkanUtilities::UnmapMemory(const VkDeviceMemory& _memoryToUnmap)
{
	vkUnmapMemory(m_MainDevice->device, _memoryToUnmap);
}

void VulkanUtilities::TransitionImageLayout(const VkImage& _image, const VkImageLayout& _oldLayout, const VkImageLayout& _newLayout, const VkPipelineStageFlagBits _startStage, const VkPipelineStageFlagBits _endStage, uint32_t _layerCount)
{
	VkCommandBuffer transitionCommandBuffer;
	BeginCommandBuffer(&transitionCommandBuffer);

	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = _oldLayout;										// Layout to transition from
	imageMemoryBarrier.newLayout = _newLayout;										// Layout to transition to
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// Queue family to transition from
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;				// Queue family to transition to
	imageMemoryBarrier.image = _image;												// Image to be modified by memory barrier
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;		// Aspect of image being altered
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;							// First mip level to start alterations on
	imageMemoryBarrier.subresourceRange.levelCount = 1;								// Number of mip levels to alter starting from baseMipLevel
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;							// First layer to start alterations on
	imageMemoryBarrier.subresourceRange.layerCount = _layerCount;					// Number of layers to alter starting from base baseArrayLayer

	if (_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = 0;								// Memory access stage transition must happen after...
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;	// Memory access stage transition must happen before...
	}
	else if (_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	
	vkCmdPipelineBarrier
	(
		transitionCommandBuffer, 
		_startStage, _endStage,							// Pipeline stages (match to src and dst AccessMasks)
		0,												// Dependency flags
		0, nullptr,										// Memory Barrier count + data
		0, nullptr,										// Buffer Memory Barrier count + data
		1, &imageMemoryBarrier							// Image Memory Barrier count + data
	);

	EndCommandBuffer(&transitionCommandBuffer);
	SubmitCommandBuffer(&transitionCommandBuffer);

	vkFreeCommandBuffers(m_MainDevice->device, *m_MainDevice->queueFamilyIndices.commandPool, 1, &transitionCommandBuffer);
}
