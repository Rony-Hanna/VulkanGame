#pragma once

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanPipelineBuilder.h"
#include "../GameObject.h"
#include "../Camera.h"
#include <string>

class Window;

class VulkanRenderer
{
public:
	VulkanRenderer(Window* _window);
	~VulkanRenderer();

	void Draw();

private:
	void CreateInstance();
	void CreateLogicalDevice();
	void CreateSurface();
	void CreateSwapchain();
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateDepthBufferImage();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateCommandBuffers();
	void CreateSynchronization();
	void CreateDescriptorPool();
	void CreateDescriptorLayout();
	void AllocateDescriptorSets();
	void WriteDescriptors();
	void CreateTextureSampler();
	void SetupScene();

	// Support 
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& _debugUtilsCreateInfo);

	// Recorders
	void RecordCommands();

	// Getters
	void SelectPhysicalDevice();
	QueueFamilyIndices GetQueueFamilyIndices(const VkPhysicalDevice& _physicalDevice);

	// Checkers
	VkBool32 CheckDeviceSuitability(const VkPhysicalDevice& _physicalDevice, uint32_t& _score);
	VkBool32 CheckRequiredDeviceExtensions(const VkPhysicalDevice& _physicalDevice);

	// Debugging
	void CreateDebugMessenger();
	void DestroyDebugMessenger();

	// Choose
	VkFormat ChooseSupportedFormat(const std::vector<VkFormat>& _formats, const VkImageTiling _tiling, const VkFormatFeatureFlags _featureFlags);

	// Updates
	void UpdateUniformBuffers();

private:
	Window* m_Window;
	VkInstance m_VkInstance;
	MainDevice m_MainDevice;
	VkDebugUtilsMessengerEXT m_DebugMessenger;
	VkSurfaceKHR m_Surface;
	VulkanSwapchain m_Swapchain;
	VkPipelineLayout m_PipelineLayout;
	VkPipeline m_GraphicsPipeline;
	VkDescriptorPool m_DescriptorPool;
	std::vector<VkDescriptorSetLayout> m_DescriptorSetLayout;
	std::vector<VkDescriptorSet> m_GlobalDescriptorSet;
	VkRenderPass m_RenderPass;
	VkCommandPool m_GraphicsCommandPool;
	VkSampler m_Sampler;
	CustomImage m_DepthImage;
	VulkanPipelineBuilder m_PipelineBuilder;

	uint32_t m_CurrentFrameIndex;
	Camera m_Camera;

	std::vector<GameObject> m_GameObjects;
	std::vector<VkFramebuffer> m_Framebuffers;
	std::vector<VkCommandBuffer> m_CommandBuffers;
	std::vector<VkSemaphore> m_WaitForImageSph;
	std::vector<VkSemaphore> m_WaitForRenderingSph;
	std::vector<VkFence> m_RenderCompleteFence;
};
