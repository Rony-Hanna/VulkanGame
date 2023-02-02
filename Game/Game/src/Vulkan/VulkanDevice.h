#pragma once

#include <vulkan/vulkan.h>
#include <vector>

struct QueueFamilyIndices
{
	int32_t graphicsFamily = -1;
	int32_t presentationFamily = -1;
	VkCommandPool* commandPool = nullptr;

	VkBool32 IsValid()
	{
		return graphicsFamily >= 0 && presentationFamily >= 0;
	}
};

struct MainDevice
{
	MainDevice() :
		device(VK_NULL_HANDLE),
		physicalDevice(VK_NULL_HANDLE),
		physicalDeviceProperties{},
		physicalDeviceFeatures{},
		queueFamilyIndices{},
		graphicsQueue(VK_NULL_HANDLE),
		presentationQueue(VK_NULL_HANDLE)
	{
		requiredDeviceExtensions.reserve(1);
		requiredDeviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkPhysicalDeviceProperties physicalDeviceProperties;
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	QueueFamilyIndices queueFamilyIndices;
	VkQueue graphicsQueue;
	VkQueue presentationQueue;
	std::vector<const char*> requiredDeviceExtensions;
};