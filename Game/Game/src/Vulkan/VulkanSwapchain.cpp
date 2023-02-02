#include "VulkanSwapchain.h"
#include "VulkanUtilities.h"
#include "VulkanInit.h"
#include <algorithm>
#include <stdexcept>

VulkanSwapchain::VulkanSwapchain() : 
	swapchainHandle(VK_NULL_HANDLE),
	m_SwapchainInfo{},
	m_SwapchainSurfaceFormat{},
	m_SwapchainImageExtent{},
	m_WidthPixel(0),
	m_HeightPixel(0)
{}

VkSwapchainCreateInfoKHR VulkanSwapchain::Init(const VkPhysicalDevice& _physicalDevice, const VkDevice& _logicalDevice, const VkSurfaceKHR& _surface, const std::pair<uint32_t, uint32_t>& _graphicsPresentQueueIndices, const int _widthInPixels, const int _heightInPixels)
{
	VulkanUtilities::GetSwapchainInfo(_physicalDevice, _surface, m_SwapchainInfo);

	m_Device = &_logicalDevice;
	m_WidthPixel = _widthInPixels;
	m_HeightPixel = _heightInPixels;

	m_SwapchainSurfaceFormat = ChooseBestSurfaceFormat();
	VkPresentModeKHR swapchainPresentMode = ChooseBestPresentMode();
	m_SwapchainImageExtent = ChooseSwapExtent();

	uint32_t imageCount = m_SwapchainInfo.surfaceCapabilities.minImageCount + 1;

	if (m_SwapchainInfo.surfaceCapabilities.maxImageCount > 0 && m_SwapchainInfo.surfaceCapabilities.maxImageCount < imageCount)
	{
		imageCount = m_SwapchainInfo.surfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = Vki::SwapchainCreateInfo();
	swapchainCreateInfo.surface = _surface;
	swapchainCreateInfo.imageFormat = m_SwapchainSurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = m_SwapchainSurfaceFormat.colorSpace;
	swapchainCreateInfo.presentMode = swapchainPresentMode;
	swapchainCreateInfo.imageExtent = m_SwapchainImageExtent;
	swapchainCreateInfo.minImageCount = imageCount;
	swapchainCreateInfo.preTransform = m_SwapchainInfo.surfaceCapabilities.currentTransform;

	// If Graphics and Presentation families are different, then swapchain must let images be shared between them
	if (_graphicsPresentQueueIndices.first != _graphicsPresentQueueIndices.second)
	{
		uint32_t queueFamilyIndices[] = { _graphicsPresentQueueIndices.first, _graphicsPresentQueueIndices.second };
	
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;		// Image share handling
		swapchainCreateInfo.queueFamilyIndexCount = 2;							// Number of queues to share images between
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;			// Array of queues to share images between
	}

	return swapchainCreateInfo;
}

void VulkanSwapchain::CleanUp(const VkDevice& _logicalDevice)
{
	for (const auto& swapchainImage : m_SwapchainImages)
	{
		vkDestroyImageView(_logicalDevice, swapchainImage.imageView, nullptr);
	}

	vkDestroySwapchainKHR(_logicalDevice, swapchainHandle, nullptr);

	m_SwapchainImages.clear();
}

std::vector<SwapchainImage> VulkanSwapchain::GetSwapchainImages() const
{
	return m_SwapchainImages;
}

VkSurfaceFormatKHR VulkanSwapchain::GetSwapchainSurfaceFormat() const
{
	return m_SwapchainSurfaceFormat;
}

VkExtent2D VulkanSwapchain::GetSwapchainImageExtent() const
{
	return m_SwapchainImageExtent;
}

void VulkanSwapchain::CreateSwapchainImageViews(const VkDevice& _logicalDevice)
{
	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(_logicalDevice, swapchainHandle, &imageCount, nullptr);
	std::vector<VkImage> swapchainImages(imageCount);
	vkGetSwapchainImagesKHR(_logicalDevice, swapchainHandle, &imageCount, swapchainImages.data());

	m_SwapchainImages.reserve(imageCount);

	for (const auto& image : swapchainImages)
	{ 
		VkImageView imageView;
		VkImageViewCreateInfo imageViewCreateInfo = Vki::ImageViewCreateInfo(image, m_SwapchainSurfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
		
		VkResult re = vkCreateImageView(_logicalDevice, &imageViewCreateInfo, nullptr, &imageView);
		if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create an image view\n");

		SwapchainImage swapchainImage{};
		swapchainImage.image = image;
		swapchainImage.imageView = imageView;
		m_SwapchainImages.emplace_back(swapchainImage);
	}
}

VkSurfaceFormatKHR VulkanSwapchain::ChooseBestSurfaceFormat()
{
	// Check to see if all surface formats are available
	if (m_SwapchainInfo.surfaceFormats.size() == 1 && m_SwapchainInfo.surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (const auto& format : m_SwapchainInfo.surfaceFormats)
	{
		if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	return m_SwapchainInfo.surfaceFormats[0];
}

VkExtent2D VulkanSwapchain::ChooseSwapExtent()
{
	// Chooses the resolution of the swapchain's images

	// If current extent is at numeric limits, then extent can vary. Otherwise, it is the size of the window
	if (m_SwapchainInfo.surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return m_SwapchainInfo.surfaceCapabilities.currentExtent;
	}

	// If value can vary, need to set manually

	VkExtent2D newExtent{};
	newExtent.width = static_cast<uint32_t>(m_WidthPixel);
	newExtent.height = static_cast<uint32_t>(m_HeightPixel);

	// Ensure extent is within boundaries
	newExtent.width = std::clamp(newExtent.width, m_SwapchainInfo.surfaceCapabilities.minImageExtent.width, m_SwapchainInfo.surfaceCapabilities.maxImageExtent.width);
	newExtent.height = std::clamp(newExtent.height, m_SwapchainInfo.surfaceCapabilities.minImageExtent.height, m_SwapchainInfo.surfaceCapabilities.maxImageExtent.height);

	return newExtent;
}

VkPresentModeKHR VulkanSwapchain::ChooseBestPresentMode()
{
	for (const auto& presentationMode : m_SwapchainInfo.surfacePresentModes)
	{
		if (presentationMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return presentationMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}
