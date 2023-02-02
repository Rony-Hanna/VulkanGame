#pragma once

#include <vulkan/vulkan.h>

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity,
													VkDebugUtilsMessageTypeFlagsEXT _messageType,
													const VkDebugUtilsMessengerCallbackDataEXT* _pCallbackData,
													void* _pUserData)
{
	if (_messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		printf("Validation layer: %s\n\n", _pCallbackData->pMessage);
	}

	return VK_FALSE;
}
