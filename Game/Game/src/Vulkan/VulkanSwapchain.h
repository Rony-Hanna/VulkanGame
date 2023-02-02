#pragma once

#include <vector>
#include <vulkan/vulkan.h>

struct SwapchainInfo
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	std::vector<VkPresentModeKHR> surfacePresentModes;
};

struct SwapchainImage
{
	VkImage image;
	VkImageView imageView;
};

class VulkanSwapchain
{
public:
	VulkanSwapchain();

	VkSwapchainCreateInfoKHR Init(const VkPhysicalDevice& _physicalDevice, const VkDevice& _logicalDevice, const VkSurfaceKHR& _surface, const std::pair<uint32_t, uint32_t>& _graphicsPresentQueueIndices, const int _widthInPixels, const int _heightInPixels);
	void CreateSwapchainImageViews(const VkDevice& _logicalDevice);
	void CleanUp(const VkDevice& _logicalDevice);
	std::vector<SwapchainImage> GetSwapchainImages() const;
	VkSurfaceFormatKHR GetSwapchainSurfaceFormat() const;
	VkExtent2D GetSwapchainImageExtent() const;

public:
	VkSwapchainKHR swapchainHandle;

private:
	VkSurfaceFormatKHR ChooseBestSurfaceFormat();
	VkExtent2D ChooseSwapExtent();
	VkPresentModeKHR ChooseBestPresentMode();

private:
	SwapchainInfo m_SwapchainInfo;
	VkSurfaceFormatKHR m_SwapchainSurfaceFormat;
	VkExtent2D m_SwapchainImageExtent;
	std::vector<SwapchainImage> m_SwapchainImages;
	const VkDevice* m_Device;
	int m_WidthPixel;
	int m_HeightPixel;
};
