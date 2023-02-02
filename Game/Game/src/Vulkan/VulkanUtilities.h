#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct MainDevice;
struct SwapchainInfo;

struct CustomImage
{
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory imageMemory;
	VkFormat imageFormat;
};

struct UniformBuffer
{
	VkBuffer buffer;
	VkDeviceMemory bufferMemory;
};

struct BufferInfo
{
	VkDeviceSize bufferSize;
	VkBufferUsageFlags bufferUsage; 
	VkMemoryPropertyFlags memoryProperties;
	VkBuffer* pBuffer; 
	VkDeviceMemory* pBufferMemory;
};

class VulkanUtilities
{
public:
	static void Initialize(const MainDevice* _mainDevice);
	static const VkDevice* GetDevice();
	static void CheckRequiredInstanceExtensions(const std::vector<const char*>& _requiredInstanceExtensions);
	static void CheckRequiredLayers(const std::vector<const char*>& _requiredLayers);
	static void CreateBuffer(BufferInfo& _bufferInfo);
	static void DestroyBuffer(const VkBuffer& _buffer, const VkDeviceMemory& _bufferMemory);
	static void DestroyImage(const VkImage& _image, const VkDeviceMemory& _imageMemory);
	static void DestroyImageView(const VkImageView& _imageView);
	static void CopyBuffer(VkBuffer& _srcBuffer, VkBuffer& _dstBuffer, const VkDeviceSize& _bufferSize);
	static void CopyBufferToImage(VkBuffer& _srcBuffer, VkImage& _dstImage, const VkExtent2D& _imageDimensions);
	static void MapMemory(const VkDeviceMemory& _memoryToMap, const VkDeviceSize& _sizeOfMemory, void** _ppData);
	static void UnmapMemory(const VkDeviceMemory& _memoryToUnmap);
	static void TransitionImageLayout(const VkImage& _image, const VkImageLayout& _oldLayout, const VkImageLayout& _newLayout, const VkPipelineStageFlagBits _startStage, const VkPipelineStageFlagBits _endStage);
	static void GetSwapchainInfo(const VkPhysicalDevice& _physicalDevice, const VkSurfaceKHR& _surface, SwapchainInfo& _swapchainInfo);
	static uint32_t FindMemoryIndex(const uint32_t _memoryTypeBits, const VkMemoryPropertyFlags& _memoryProperties);
	static VkShaderModule CreateShaderModule(const std::vector<char>& _shaderCode);
	static CustomImage CreateImage(const VkExtent2D& _dimensions, const VkFormat _format, const VkImageUsageFlags _usage, const VkMemoryPropertyFlags _memoryProperty);

private:
	static void BeginCommandBuffer(VkCommandBuffer* _commandBuffer);
	static void EndCommandBuffer(VkCommandBuffer* _commandBuffer);
	static void SubmitCommandBuffer(VkCommandBuffer* _commandBuffer);

private:
	static const MainDevice* m_MainDevice;
};