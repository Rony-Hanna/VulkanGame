#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Vki
{
	VkApplicationInfo AppInfo(const char* _appName, const uint32_t _appVersion, const char* _engineName, const uint32_t _engineVersion, const uint32_t _vulkanVersion);
	VkInstanceCreateInfo InstanceCreateInfo(const VkApplicationInfo& _appInfo, const std::vector<const char*>& _instanceExtensions, const std::vector<const char*>& _layersToEnable = {});	
	VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo(const VkInstance& _vkInstance);
	VkDeviceQueueCreateInfo QueueCreateInfo(const uint32_t _queueCount, const uint32_t _queueFamilyIndex);
	VkDeviceCreateInfo DeviceCreateInfo(const VkPhysicalDeviceFeatures& _deviceFeatures, const std::vector<VkDeviceQueueCreateInfo>& _queueCreateInfo, const std::vector<const char*>& _deviceExtensions);
	VkSwapchainCreateInfoKHR SwapchainCreateInfo();
	VkShaderModuleCreateInfo ShaderModuleCreateInfo(const std::vector<char>& _shaderCode);
	VkPipelineShaderStageCreateInfo ShaderStageCreateInfo(const VkShaderStageFlagBits _shaderStage, const VkShaderModule& _shaderModule);
	VkVertexInputBindingDescription VertexInputBindingDescription(const uint32_t _binding, const uint32_t _stride, const VkVertexInputRate _inputRate);
	VkVertexInputAttributeDescription VertexInputAttributeDescription(const uint32_t _binding, const uint32_t _location, const VkFormat _format, const uint32_t _offset);
	VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo(const uint32_t _bindingCount, const VkVertexInputBindingDescription& _bindingDescription, const std::vector<VkVertexInputAttributeDescription>& _vertexAttributes);
	VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo(const VkPrimitiveTopology _topology, const VkBool32 _restartPrimitive);
	VkPipelineViewportStateCreateInfo ViewportStateCreateInfo(const uint32_t _viewportCount, const VkViewport& _viewport, const uint32_t _scissorCount, const VkRect2D& _scissor);
	VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(const VkPolygonMode _polyMode, const VkCullModeFlagBits _cullMode, const VkFrontFace _frontFace);
	VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo(const VkBool32 _enableSampling, const VkSampleCountFlagBits _samples);
	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState(const VkBool32 _enableBlend, const VkColorComponentFlags _colorWriteMask);
	VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo(const VkBool32 _logicOpEnable, const uint32_t _attachmentCount, const VkPipelineColorBlendAttachmentState& _attachment);
	VkPipelineLayoutCreateInfo LayoutCreateInfo();
	VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo(const VkBool32 _enableDepthTest, const VkBool32 _enableDepthWrite, const VkCompareOp _compareOp, const VkBool32 _enableDepthBoundsTest, const VkBool32 _enableStencilTest);
	VkViewport ViewportInfo(const VkExtent2D& _viewportSize);
	VkRect2D ScissorInfo(const VkExtent2D& _scissorSize);
	VkPushConstantRange PushConstantRange(const VkShaderStageFlags _shaderStage, const uint32_t _offset, const uint32_t _size);
	VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo();
	VkImageCreateInfo ImageCreateInfo();
	VkImageViewCreateInfo ImageViewCreateInfo(const VkImage& _image, const VkFormat _format, const VkImageAspectFlags _aspectFlags);
	VkRenderPassCreateInfo RenderPassCreateInfo();
	VkFramebufferCreateInfo FramebufferCreateInfo(const VkExtent2D& _framebufferSize);
	VkSamplerCreateInfo SamplerCreateInfo(const VkBool32 _enableAnisotropy, const float _maxAnisotropy = 1.0f);
	VkCommandPoolCreateInfo CommandPoolCreateInfo(const uint32_t _queueFamilyIndex);
	VkCommandBufferAllocateInfo AllocateCommandBuffer(const VkCommandPool& _commandPool, const uint32_t _bufferCount, const VkCommandBufferLevel _level);
	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& _poolSizes, const uint32_t _maxSets);
	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(const uint32_t _binding, const VkDescriptorType _descriptorType, const VkShaderStageFlags _shaderStage, const uint32_t _descriptorCount);
	VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutBinding& _layoutBinding, const uint32_t _bindingCount);
	VkDescriptorSetAllocateInfo AllocateDescriptorSet(const uint32_t _setCount, const VkDescriptorPool& _pool, const VkDescriptorSetLayout& _setLayout);
	VkBufferCreateInfo BufferCreateInfo(const VkBufferUsageFlags _usage, const VkDeviceSize _bufferSize);
	VkMemoryAllocateInfo AllocateMemoryInfo(const VkDeviceSize _memorySize, const uint32_t _memoryTypeIndex);
}