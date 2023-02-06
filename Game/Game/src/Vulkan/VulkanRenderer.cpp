#include "VulkanRenderer.h"
#include "VulkanInit.h"
#include "../Window.h"
#include "VulkanDebug.h"
#include "../Utilities.h"
#include <glfw3.h>
#include <array>
#include <algorithm>
#include <stdexcept>

#define MaxFrameDraws 3
#define MaxObjects 25

VulkanRenderer::VulkanRenderer(Window* _window) :
	m_Window(_window),
	m_VkInstance(VK_NULL_HANDLE),
	m_MainDevice{},
	m_DebugMessenger(VK_NULL_HANDLE),
	m_Surface(VK_NULL_HANDLE),
	m_Swapchain{},
	m_GraphicsPipeline(VK_NULL_HANDLE),
	m_PipelineLayout(VK_NULL_HANDLE),
	m_CurrentFrameIndex(0)
{
	CreateInstance();
	if (enableValidationLayers) CreateDebugMessenger();
	CreateSurface();
	SelectPhysicalDevice();
	CreateLogicalDevice();

	VulkanUtilities::Initialize(&m_MainDevice);
	m_Camera.Init();

	CreateSwapchain();
	CreateDepthBufferImage();
	CreateRenderPass();
	CreateFramebuffers();
	CreateCommandPool();
	CreateCommandBuffers();
	CreateSynchronization();
	CreateTextureSampler();
	CreateDescriptorPool();
	CreateDescriptorLayout();
	AllocateDescriptorSets();
	CreateGraphicsPipeline();
	CreateGraphicsPipeline2();

	SetupScene();

	WriteDescriptors();
	RecordCommands();
}

VulkanRenderer::~VulkanRenderer()
{
	// Wait until no actions being run on device before destroying
	vkDeviceWaitIdle(m_MainDevice.device);

	vkDestroySampler(m_MainDevice.device, m_Sampler, nullptr);

	VulkanUtilities::DestroyImageView(m_DepthImage.imageView);
	VulkanUtilities::DestroyImage(m_DepthImage.image, m_DepthImage.imageMemory);

	vkDestroyDescriptorPool(m_MainDevice.device, m_DescriptorPool, nullptr);

	// Destroy descriptor set layouts
	for (const auto& descriptorSetLayout : m_DescriptorSetLayout)
	{
		vkDestroyDescriptorSetLayout(m_MainDevice.device, descriptorSetLayout, nullptr);
	}

	m_Camera.CleanUp();
	m_Skybox.CleanUp();

	for (auto& gameObject : m_GameObjects)
	{
		gameObject.CleanUp();
	}

	for (uint8_t i = 0; i < MaxFrameDraws; ++i)
	{
		vkDestroyFence(m_MainDevice.device, m_RenderCompleteFence[i], nullptr);
		vkDestroySemaphore(m_MainDevice.device, m_WaitForRenderingSph[i], nullptr);
		vkDestroySemaphore(m_MainDevice.device, m_WaitForImageSph[i], nullptr);
	}
	
	vkDestroyCommandPool(m_MainDevice.device, m_GraphicsCommandPool, nullptr);

	for (const auto& framebuffer : m_Framebuffers)
	{
		vkDestroyFramebuffer(m_MainDevice.device, framebuffer, nullptr);
	}

	m_Framebuffers.clear();

	vkDestroyRenderPass(m_MainDevice.device, m_RenderPass, nullptr);

	vkDestroyPipeline(m_MainDevice.device, m_GraphicsPipeline2, nullptr);
	vkDestroyPipelineLayout(m_MainDevice.device, m_PipelineLayout2, nullptr);

	vkDestroyPipeline(m_MainDevice.device, m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_MainDevice.device, m_PipelineLayout, nullptr);

	m_Swapchain.CleanUp(m_MainDevice.device);

	vkDestroySurfaceKHR(m_VkInstance, m_Surface, nullptr);
	vkDestroyDevice(m_MainDevice.device, nullptr);

	if (enableValidationLayers)
	{
		DestroyDebugMessenger();
	}

	vkDestroyInstance(m_VkInstance, nullptr);
}

void VulkanRenderer::Draw()
{
	vkWaitForFences(m_MainDevice.device, 1, &m_RenderCompleteFence[m_CurrentFrameIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(m_MainDevice.device, 1, &m_RenderCompleteFence[m_CurrentFrameIndex]);

	// -- Get Next Image --
	// Get index of next image to be drawn to, and signal semaphore when ready to be drawn to
	uint32_t imageIndex = 0;
	VkResult re = vkAcquireNextImageKHR(m_MainDevice.device, m_Swapchain.swapchainHandle, std::numeric_limits<uint64_t>::max(), m_WaitForImageSph[m_CurrentFrameIndex], VK_NULL_HANDLE, &imageIndex);

	UpdateUniformBuffers();

	// -- Submit Command Buffer To Render --
	VkPipelineStageFlags waitStages[] =
	{
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;												// Number of semaphores to wait on
	submitInfo.pWaitSemaphores = &m_WaitForImageSph[m_CurrentFrameIndex];			// List of semaphores to wait on
	submitInfo.pWaitDstStageMask = waitStages;										// Stages to check semaphores at
	submitInfo.commandBufferCount = 1;												// Number of command buffers to submit
	submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];						// Command buffer to submit
	submitInfo.signalSemaphoreCount = 1;											// Number of semaphores to signal
	submitInfo.pSignalSemaphores = &m_WaitForRenderingSph[m_CurrentFrameIndex];		// Semaphores to signal when command buffer finishes
	
	re = vkQueueSubmit(m_MainDevice.graphicsQueue, 1, &submitInfo, m_RenderCompleteFence[m_CurrentFrameIndex]);

	if (re != VK_SUCCESS)
	{
		throw std::runtime_error("VULKAN ERROR: Failed to submit command buffer to queue\n");
	}

	// -- Present Rendered Image To Screen --
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;												// Number of semaphores to wait on
	presentInfo.pWaitSemaphores = &m_WaitForRenderingSph[m_CurrentFrameIndex];		// Semaphores to wait on
	presentInfo.swapchainCount = 1;													// Number of swapchains to present to
	presentInfo.pSwapchains = &m_Swapchain.swapchainHandle;											// Swapchains to present images to
	presentInfo.pImageIndices = &imageIndex;										// Index of images in swapchain to present

	re = vkQueuePresentKHR(m_MainDevice.graphicsQueue, &presentInfo);

	if (re != VK_SUCCESS)
	{
		throw std::runtime_error("VULKAN ERROR: Failed to submit command buffer to queue\n");
	}

	m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % MaxFrameDraws;
}

void VulkanRenderer::CreateInstance()
{
	VkApplicationInfo appInfo = Vki::AppInfo("Voyager Deep", VK_MAKE_VERSION(1, 0, 0), "Banshee", VK_MAKE_VERSION(1, 0, 0), VK_API_VERSION_1_3);

	// Get required vulkan instance extensions from glfw
	uint32_t requiredInstanceExtensionCount = 0;
	const char** glfwRequiredExtensions = glfwGetRequiredInstanceExtensions(&requiredInstanceExtensionCount);

	std::vector<const char*> requiredExtensions;
	requiredExtensions.reserve(requiredInstanceExtensionCount);

	for (uint16_t i = 0; i < requiredInstanceExtensionCount; ++i)
	{
		requiredExtensions.emplace_back(glfwRequiredExtensions[i]);
	}

	VulkanUtilities::CheckRequiredInstanceExtensions(requiredExtensions);

	std::vector<const char*> validationLayers;

	if (enableValidationLayers)
	{
		validationLayers.reserve(1);
		validationLayers.emplace_back("VK_LAYER_KHRONOS_validation");

		VulkanUtilities::CheckRequiredLayers(validationLayers);

		requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = Vki::DebugUtilsMessengerCreateInfo(m_VkInstance);
		VkInstanceCreateInfo instanceCreateInfo = Vki::InstanceCreateInfo(appInfo, requiredExtensions, validationLayers);
		instanceCreateInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debugMessengerCreateInfo);

		VkResult re = vkCreateInstance(&instanceCreateInfo, nullptr, &m_VkInstance);
		if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create a vulkan instance\n");
	}
	else
	{
		VkInstanceCreateInfo instanceCreateInfo = Vki::InstanceCreateInfo(appInfo, requiredExtensions);

		VkResult re = vkCreateInstance(&instanceCreateInfo, nullptr, &m_VkInstance);
		if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create a vulkan instance\n");
	}
}

void VulkanRenderer::CreateDebugMessenger()
{
	VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = Vki::DebugUtilsMessengerCreateInfo(m_VkInstance);
	PopulateDebugMessengerCreateInfo(debugMessengerCreateInfo);

	auto createDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_VkInstance, "vkCreateDebugUtilsMessengerEXT");
	if (!createDebugUtilsMessenger)
	{
		throw std::runtime_error("VULKAN ERROR: Failed to retrieve vkCreateDebugUtilsMessengerEXT function pointer\n");
	}

	VkResult re = createDebugUtilsMessenger(m_VkInstance, &debugMessengerCreateInfo, nullptr, &m_DebugMessenger);
	if (re != VK_SUCCESS) throw std::runtime_error("ERROR: Failed to create debug messenger callback\n");
}

void VulkanRenderer::DestroyDebugMessenger()
{
	auto destroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_VkInstance, "vkDestroyDebugUtilsMessengerEXT");
	if (!destroyDebugUtilsMessenger)
	{
		throw std::runtime_error("VULKAN ERROR: Failed to retrieve vkDestroyDebugUtilsMessengerEXT function pointer\n");
	}

	destroyDebugUtilsMessenger(m_VkInstance, m_DebugMessenger, nullptr);
}

void VulkanRenderer::SelectPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, nullptr);
	std::vector<VkPhysicalDevice> availablePhysicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(m_VkInstance, &deviceCount, availablePhysicalDevices.data());

	uint32_t bestScore = 0;

	for (auto& device : availablePhysicalDevices)
	{
		uint32_t score = 0;

		if (CheckDeviceSuitability(device, score))
		{
			if (bestScore <= score)
			{
				bestScore = score;
				m_MainDevice.physicalDevice = device;
			}
		}
	}

	if (!m_MainDevice.physicalDevice)
	{
		throw std::runtime_error("VULKAN ERROR: Failed to find a suitable GPU device\n");
	}
}

void VulkanRenderer::CreateLogicalDevice()
{
	m_MainDevice.queueFamilyIndices = GetQueueFamilyIndices(m_MainDevice.physicalDevice);
	std::vector<int> uniqueQueueFamilies = { m_MainDevice.queueFamilyIndices.graphicsFamily, m_MainDevice.queueFamilyIndices.presentationFamily };

	// Remove duplicate indices from vector
	std::sort(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());
	auto last = std::unique(uniqueQueueFamilies.begin(), uniqueQueueFamilies.end());
	uniqueQueueFamilies.erase(last, uniqueQueueFamilies.end());

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	for (const auto& queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo deviceQueueCreateInfo = Vki::QueueCreateInfo(1, queueFamily);
		queueCreateInfos.emplace_back(deviceQueueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo = Vki::DeviceCreateInfo(deviceFeatures, queueCreateInfos, m_MainDevice.requiredDeviceExtensions);
	
	VkResult re = vkCreateDevice(m_MainDevice.physicalDevice, &deviceCreateInfo, nullptr, &m_MainDevice.device);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create logical device\n");

	vkGetDeviceQueue(m_MainDevice.device, m_MainDevice.queueFamilyIndices.graphicsFamily, 0, &m_MainDevice.graphicsQueue);
	vkGetDeviceQueue(m_MainDevice.device, m_MainDevice.queueFamilyIndices.presentationFamily, 0, &m_MainDevice.presentationQueue);

	vkGetPhysicalDeviceProperties(m_MainDevice.physicalDevice, &m_MainDevice.physicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(m_MainDevice.physicalDevice, &m_MainDevice.physicalDeviceFeatures);
}

VkFormat VulkanRenderer::ChooseSupportedFormat(const std::vector<VkFormat>& _formats, const VkImageTiling _tiling, const VkFormatFeatureFlags _featureFlags)
{
	for (const auto& format : _formats)
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(m_MainDevice.physicalDevice, format, &formatProperties);

		// Depending on tiling choice, need to check for different bit flag
		if (_tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & _featureFlags) == _featureFlags)
		{
			return format;
		}
		else if (_tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & _featureFlags) == _featureFlags)
		{
			return format;
		}
	}

	throw std::runtime_error("VULKAN ERROR: Failed to find a matching format\n");
}

void VulkanRenderer::UpdateUniformBuffers()
{
	m_Camera.Update();
}

VkBool32 VulkanRenderer::CheckDeviceSuitability(const VkPhysicalDevice& _physicalDevice, uint32_t& _score)
{
	if (!CheckRequiredDeviceExtensions(_physicalDevice)) return false;

	QueueFamilyIndices queueFamilyIndices = GetQueueFamilyIndices(_physicalDevice);
	if (!queueFamilyIndices.IsValid()) return false;

	SwapchainInfo swapchainInfo{};
	VulkanUtilities::GetSwapchainInfo(_physicalDevice, m_Surface, swapchainInfo);
	if (swapchainInfo.surfaceFormats.empty() || swapchainInfo.surfacePresentModes.empty()) return false;

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;

	vkGetPhysicalDeviceProperties(_physicalDevice, &deviceProperties);
	vkGetPhysicalDeviceFeatures(_physicalDevice, &deviceFeatures);

	if (deviceFeatures.samplerAnisotropy)
	{
		_score += 1000;
	}

	if (deviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		_score += 1000;
	}

	_score += deviceProperties.limits.maxImageDimension2D;

	return true;
}

VkBool32 VulkanRenderer::CheckRequiredDeviceExtensions(const VkPhysicalDevice& _physicalDevice)
{
	uint32_t deviceExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &deviceExtensionCount, nullptr);
	if (deviceExtensionCount == 0) return false;
	std::vector<VkExtensionProperties> availableDeviceExtensions(deviceExtensionCount);
	vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &deviceExtensionCount, availableDeviceExtensions.data());

	for (const auto& requiredDeviceExtension : m_MainDevice.requiredDeviceExtensions)
	{
		bool foundExtension = false;

		for (const auto& availableDeviceExtension : availableDeviceExtensions)
		{
			if (std::strcmp(requiredDeviceExtension, availableDeviceExtension.extensionName) == 0)
			{
				foundExtension = true;
				break;
			}
		}

		if (!foundExtension)
		{
			return false;
		}
	}

	return true;
}

QueueFamilyIndices VulkanRenderer::GetQueueFamilyIndices(const VkPhysicalDevice& _physicalDevice)
{
	QueueFamilyIndices queueFamilyIndices{};

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> availableQueueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, availableQueueFamilies.data());

	int index = 0;

	for (const auto& queueFamily : availableQueueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueFamilyIndices.graphicsFamily = index;
		}

		VkBool32 presentationSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, static_cast<uint32_t>(index), m_Surface, &presentationSupport);
		
		if (queueFamily.queueCount > 0 && presentationSupport)
		{
			queueFamilyIndices.presentationFamily = index;
		}

		if (queueFamilyIndices.IsValid())
		{
			break;
		}

		++index;
	}

	return queueFamilyIndices;
}

void VulkanRenderer::CreateSurface()
{
	VkResult re = glfwCreateWindowSurface(m_VkInstance, m_Window->GetWindow(), nullptr, &m_Surface);

	if (re != VK_SUCCESS)
	{
		throw std::runtime_error("VULKAN ERROR: Failed to create vulkan surface\n");
	}
}

void VulkanRenderer::CreateSwapchain()
{
	int width, height = 0;
	glfwGetFramebufferSize(m_Window->GetWindow(), &width, &height);

	VkSwapchainCreateInfoKHR swapchainCreateInfo = m_Swapchain.Init(m_MainDevice.physicalDevice, m_MainDevice.device, m_Surface, std::make_pair(m_MainDevice.queueFamilyIndices.graphicsFamily, m_MainDevice.queueFamilyIndices.presentationFamily), width, height);

	VkResult re = vkCreateSwapchainKHR(m_MainDevice.device, &swapchainCreateInfo, nullptr, &m_Swapchain.swapchainHandle);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create swapchain\n");

	m_Swapchain.CreateSwapchainImageViews(m_MainDevice.device);
}

void VulkanRenderer::CreateGraphicsPipeline()
{
	// -- Shader Creation --
	const auto vertexShaderCode = Utilities::ReadBinaryFile("src/Shaders/vert.spv");
	const auto fragmentShaderCode = Utilities::ReadBinaryFile("src/Shaders/frag.spv");

	VkShaderModule vertexShaderModule = VulkanUtilities::CreateShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = VulkanUtilities::CreateShaderModule(fragmentShaderCode);
	
	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = Vki::ShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShaderModule);
	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = Vki::ShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShaderModule);
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

	// -- Vertex Input --
	VkVertexInputBindingDescription bindingDescription = Vki::VertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

	std::vector<VkVertexInputAttributeDescription> vertexAttributesDescriptions{};
	vertexAttributesDescriptions.resize(3);

	vertexAttributesDescriptions[0] = Vki::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
	vertexAttributesDescriptions[1] = Vki::VertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
	vertexAttributesDescriptions[2] = Vki::VertexInputAttributeDescription(0, 2, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv));

	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = Vki::VertexInputStateCreateInfo(1, bindingDescription, vertexAttributesDescriptions);

	// -- Input Assembly --
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = Vki::InputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

	// -- Viewport & Scissor --
	VkViewport viewport = Vki::ViewportInfo(m_Swapchain.GetSwapchainImageExtent());
	VkRect2D scissor = Vki::ScissorInfo(m_Swapchain.GetSwapchainImageExtent());
	VkPipelineViewportStateCreateInfo viewportCreateInfo = Vki::ViewportStateCreateInfo(1, viewport, 1, scissor);

	// -- Rasterizer --
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = Vki::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE);

	// -- Multisampling (anti-aliasing) --
	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = Vki::MultisampleStateCreateInfo(VK_FALSE, VK_SAMPLE_COUNT_1_BIT);

	// -- Blending --
	VkPipelineColorBlendAttachmentState colorBlendState = Vki::ColorBlendAttachmentState(VK_TRUE, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
	colorBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = Vki::ColorBlendStateCreateInfo(VK_FALSE, 1, colorBlendState);

	// -- Pipeline layout --
	VkPushConstantRange vertexPushConstants = Vki::PushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ObjectData));
	
	VkPipelineLayoutCreateInfo layoutCreateInfo = Vki::LayoutCreateInfo();
	layoutCreateInfo.pushConstantRangeCount = 1;
	layoutCreateInfo.pPushConstantRanges = &vertexPushConstants;
	layoutCreateInfo.setLayoutCount = static_cast<uint32_t>(m_DescriptorSetLayout.size());
	layoutCreateInfo.pSetLayouts = m_DescriptorSetLayout.data();

	VkResult re = vkCreatePipelineLayout(m_MainDevice.device, &layoutCreateInfo, nullptr, &m_PipelineLayout);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create pipeline layout\n");

	// -- Depth & Stencil testing --
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = Vki::DepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS, VK_FALSE, VK_FALSE);

	// -- Graphics pipeline --
	m_PipelineBuilder.shaderStages = { vertexShaderCreateInfo, fragmentShaderCreateInfo };
	m_PipelineBuilder.vertexInputStateCreateInfo = vertexInputCreateInfo;
	m_PipelineBuilder.inputAssemblyStateCreateInfo = inputAssemblyCreateInfo;
	m_PipelineBuilder.viewportStateCreateInfo = viewportCreateInfo;
	m_PipelineBuilder.rasterizationStateCreateInfo = rasterizerCreateInfo;
	m_PipelineBuilder.multisampleStateCreateInfo = multisampleCreateInfo;
	m_PipelineBuilder.colorBlendStateCreateInfo = colorBlendCreateInfo;
	m_PipelineBuilder.depthStencilStateCreateInfo = depthStencilCreateInfo;
	m_PipelineBuilder.layout = m_PipelineLayout;

	m_GraphicsPipeline = m_PipelineBuilder.Build(m_RenderPass, m_MainDevice.device);

	vkDestroyShaderModule(m_MainDevice.device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(m_MainDevice.device, fragmentShaderModule, nullptr);
}

void VulkanRenderer::CreateGraphicsPipeline2()
{
	// -- Shader Creation --
	const auto vertexShaderCode = Utilities::ReadBinaryFile("src/Shaders/skyboxVert.spv");
	const auto fragmentShaderCode = Utilities::ReadBinaryFile("src/Shaders/skyboxFrag.spv");

	VkShaderModule vertexShaderModule = VulkanUtilities::CreateShaderModule(vertexShaderCode);
	VkShaderModule fragmentShaderModule = VulkanUtilities::CreateShaderModule(fragmentShaderCode);

	VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = Vki::ShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexShaderModule);
	VkPipelineShaderStageCreateInfo fragmentShaderCreateInfo = Vki::ShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentShaderModule);
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertexShaderCreateInfo, fragmentShaderCreateInfo };

	// -- Vertex Input --
	VkVertexInputBindingDescription bindingDescription = Vki::VertexInputBindingDescription(0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX);

	std::vector<VkVertexInputAttributeDescription> vertexAttributesDescriptions{};
	vertexAttributesDescriptions.resize(1);
	vertexAttributesDescriptions[0] = Vki::VertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
	VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = Vki::VertexInputStateCreateInfo(1, bindingDescription, vertexAttributesDescriptions);

	// -- Input Assembly --
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo = Vki::InputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE);

	// -- Viewport & Scissor --
	VkViewport viewport = Vki::ViewportInfo(m_Swapchain.GetSwapchainImageExtent());
	VkRect2D scissor = Vki::ScissorInfo(m_Swapchain.GetSwapchainImageExtent());
	VkPipelineViewportStateCreateInfo viewportCreateInfo = Vki::ViewportStateCreateInfo(1, viewport, 1, scissor);

	// -- Rasterizer --
	VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo = Vki::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);

	// -- Multisampling (anti-aliasing) --
	VkPipelineMultisampleStateCreateInfo multisampleCreateInfo = Vki::MultisampleStateCreateInfo(VK_FALSE, VK_SAMPLE_COUNT_1_BIT);

	// -- Blending --
	VkPipelineColorBlendAttachmentState colorBlendState = Vki::ColorBlendAttachmentState(VK_TRUE, VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
	colorBlendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendState.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo = Vki::ColorBlendStateCreateInfo(VK_FALSE, 1, colorBlendState);

	// -- Pipeline layout --
	VkPipelineLayoutCreateInfo layoutCreateInfo = Vki::LayoutCreateInfo();
	layoutCreateInfo.pushConstantRangeCount = 0;
	layoutCreateInfo.pPushConstantRanges = nullptr;
	layoutCreateInfo.setLayoutCount = static_cast<uint32_t>(m_SkyboxDescriptorSetLayout.size());
	layoutCreateInfo.pSetLayouts = m_SkyboxDescriptorSetLayout.data();

	VkResult re = vkCreatePipelineLayout(m_MainDevice.device, &layoutCreateInfo, nullptr, &m_PipelineLayout2);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create pipeline layout\n");

	// -- Depth & Stencil testing --
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = Vki::DepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS, VK_FALSE, VK_FALSE);

	// -- Graphics pipeline --
	m_PipelineBuilder.shaderStages = { vertexShaderCreateInfo, fragmentShaderCreateInfo };
	m_PipelineBuilder.vertexInputStateCreateInfo = vertexInputCreateInfo;
	m_PipelineBuilder.inputAssemblyStateCreateInfo = inputAssemblyCreateInfo;
	m_PipelineBuilder.viewportStateCreateInfo = viewportCreateInfo;
	m_PipelineBuilder.rasterizationStateCreateInfo = rasterizerCreateInfo;
	m_PipelineBuilder.multisampleStateCreateInfo = multisampleCreateInfo;
	m_PipelineBuilder.colorBlendStateCreateInfo = colorBlendCreateInfo;
	m_PipelineBuilder.depthStencilStateCreateInfo = depthStencilCreateInfo;
	m_PipelineBuilder.layout = m_PipelineLayout2;

	m_GraphicsPipeline2 = m_PipelineBuilder.Build(m_RenderPass, m_MainDevice.device);

	vkDestroyShaderModule(m_MainDevice.device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(m_MainDevice.device, fragmentShaderModule, nullptr);
}

void VulkanRenderer::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_Swapchain.GetSwapchainSurfaceFormat().format;	// Format to use for attachment
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;							// Number of sample to write for multisampling
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;						// Describes what to do with the attachment before rendering
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// Describes what to do with the attachment after rendering
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;			// Describes what to do with stencil before rending
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;			// Describes what to do with stencil after rendering

	VkAttachmentDescription depthStencilAttachment{};
	depthStencilAttachment.format = m_DepthImage.imageFormat;
	depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Framebuffer data will be stored as an image, but images can be given different data layouts
	// to give optimal use for certain operations
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;			// Image data layout before render pass starts
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;		// Image data layout after render pass finishes (to change to)

	depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Attachment reference uses an attachment index that refers to index in the attachment list passed to renderPassCreateInfo
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthStencilAttachmentRef{};
	depthStencilAttachmentRef.attachment = 1;
	depthStencilAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Information about a particular subpass the Render Pass is using
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;		// Which pipeline type to bind to
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthStencilAttachmentRef;

	std::array<VkSubpassDependency, 2> subpassDependencies{};

	// Conversion from VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
	// Transition must happen after...
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;								// Subpass index (VK_SUBPASS_EXTERNAL = special value meaning outside of render pass)
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;				// Pipeline stage
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;						// Stage access mask (memory access)

	// But must happen before...
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Conversion from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	// Transition must happen after...
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// But must happen before...
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

	std::array<VkAttachmentDescription, 2> attachments{ colorAttachment, depthStencilAttachment };

	VkRenderPassCreateInfo renderPassCreateInfo = Vki::RenderPassCreateInfo();
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassCreateInfo.pAttachments = attachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	VkResult re = vkCreateRenderPass(m_MainDevice.device, &renderPassCreateInfo, nullptr, &m_RenderPass);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create render pass\n");
}

void VulkanRenderer::CreateDepthBufferImage()
{
	std::vector<VkFormat> desiredFormats { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT };
	VkFormat depthImageFormat = ChooseSupportedFormat(desiredFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	m_DepthImage = VulkanUtilities::CreateImage(m_Swapchain.GetSwapchainImageExtent(), depthImageFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VkImageViewCreateInfo imageViewCreateInfo = Vki::ImageViewCreateInfo(m_DepthImage.image, depthImageFormat, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);

	VkResult re = vkCreateImageView(m_MainDevice.device, &imageViewCreateInfo, nullptr, &m_DepthImage.imageView);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create an image view\n");
}

void VulkanRenderer::CreateFramebuffers()
{
	std::vector<SwapchainImage> swapchainImages = m_Swapchain.GetSwapchainImages();

	m_Framebuffers.resize(swapchainImages.size());

	// Create a framebuffer for each image in swapchain
	for (uint16_t i = 0; i < swapchainImages.size(); ++i)
	{
		std::array<VkImageView, 2> attachments =
		{
			swapchainImages[i].imageView,
			m_DepthImage.imageView
		};

		VkFramebufferCreateInfo framebufferCreateInfo = Vki::FramebufferCreateInfo(m_Swapchain.GetSwapchainImageExtent());
		framebufferCreateInfo.renderPass = m_RenderPass;										// RenderPass layout the Framebuffer will be used with
		framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferCreateInfo.pAttachments = attachments.data();								// List of attachments (1:1 with RenderPass)

		VkResult re = vkCreateFramebuffer(m_MainDevice.device, &framebufferCreateInfo, nullptr, &m_Framebuffers[i]);
		if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create framebuffers\n");
	}
}

void VulkanRenderer::CreateCommandPool()
{
	VkCommandPoolCreateInfo commandPoolInfo = Vki::CommandPoolCreateInfo(m_MainDevice.queueFamilyIndices.graphicsFamily);

	VkResult re = vkCreateCommandPool(m_MainDevice.device, &commandPoolInfo, nullptr, &m_GraphicsCommandPool);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create command pool\n");

	m_MainDevice.queueFamilyIndices.commandPool = &m_GraphicsCommandPool;
}

void VulkanRenderer::CreateCommandBuffers()
{
	const uint32_t bufferCount = static_cast<uint32_t>(m_Framebuffers.size());
	m_CommandBuffers.resize(bufferCount);

	VkCommandBufferAllocateInfo bufferAllocateInfo = Vki::AllocateCommandBuffer(m_GraphicsCommandPool, bufferCount, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

	VkResult re = vkAllocateCommandBuffers(m_MainDevice.device, &bufferAllocateInfo, m_CommandBuffers.data());
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to allocate command buffers\n");
}

void VulkanRenderer::CreateSynchronization()
{
	m_WaitForImageSph.resize(MaxFrameDraws);
	m_WaitForRenderingSph.resize(MaxFrameDraws);
	m_RenderCompleteFence.resize(MaxFrameDraws);

	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint8_t i = 0; i < MaxFrameDraws; ++i)
	{
		if (vkCreateSemaphore(m_MainDevice.device, &semaphoreCreateInfo, nullptr, &m_WaitForImageSph[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("VULKAN ERROR: Failed to create ImageAvailable semaphore\n");
		}

		if (vkCreateSemaphore(m_MainDevice.device, &semaphoreCreateInfo, nullptr, &m_WaitForRenderingSph[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("VULKAN ERROR: Failed to create RenderFinished semaphore\n");
		}

		if (vkCreateFence(m_MainDevice.device, &fenceCreateInfo, nullptr, &m_RenderCompleteFence[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("VULKAN ERROR: Failed to create Render Complete fence\n");
		}
	}
}

void VulkanRenderer::CreateDescriptorPool()
{
	std::vector<VkDescriptorPoolSize> poolSizes = 
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1},
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 2},
	};

	VkDescriptorPoolCreateInfo poolCreateInfo = Vki::DescriptorPoolCreateInfo(poolSizes, 3);

	auto re = vkCreateDescriptorPool(m_MainDevice.device, &poolCreateInfo, nullptr, &m_DescriptorPool);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create description pool\n");

	// ----------TEMP----------
	std::vector<VkDescriptorPoolSize> poolSizes2 =
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
	};

	VkDescriptorPoolCreateInfo poolCreateInfo2 = Vki::DescriptorPoolCreateInfo(poolSizes, 2);

	re = vkCreateDescriptorPool(m_MainDevice.device, &poolCreateInfo2, nullptr, &m_SkyboxDescriptorPool);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create description pool\n");
	// ------------------------
}

void VulkanRenderer::CreateDescriptorLayout()
{
	m_DescriptorSetLayout.resize(3);

	// Configure Uniform Buffer descriptor set layout
	VkDescriptorSetLayoutBinding uboLayoutBinding = Vki::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
	VkDescriptorSetLayoutCreateInfo uboSetLayoutCreateInfo = Vki::DescriptorSetLayoutCreateInfo(uboLayoutBinding, 1);

	auto re = vkCreateDescriptorSetLayout(m_MainDevice.device, &uboSetLayoutCreateInfo, nullptr, &m_DescriptorSetLayout[0]);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create description set layout\n");

	// Configure Sampler descriptor set layout
	VkDescriptorSetLayoutBinding samplerLayoutBinding = Vki::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutCreateInfo samplerSetLayoutCreateInfo = Vki::DescriptorSetLayoutCreateInfo(samplerLayoutBinding, 1);

	re = vkCreateDescriptorSetLayout(m_MainDevice.device, &samplerSetLayoutCreateInfo, nullptr, &m_DescriptorSetLayout[1]);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create description set layout\n");

	// Configure Texture descriptor set layout
	VkDescriptorSetLayoutBinding textureLayoutBinding = Vki::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 2);
	VkDescriptorSetLayoutCreateInfo textureSetLayoutCreateInfo = Vki::DescriptorSetLayoutCreateInfo(textureLayoutBinding, 1);

	re = vkCreateDescriptorSetLayout(m_MainDevice.device, &textureSetLayoutCreateInfo, nullptr, &m_DescriptorSetLayout[2]);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create description set layout\n");

	// ----------TEMP----------
	m_SkyboxDescriptorSetLayout.resize(2);

	// Configure Uniform Buffer descriptor set layout
	VkDescriptorSetLayoutBinding uboSkyboxLayoutBinding = Vki::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1);
	VkDescriptorSetLayoutCreateInfo uboSkyboxSetLayoutCreateInfo = Vki::DescriptorSetLayoutCreateInfo(uboSkyboxLayoutBinding, 1);

	re = vkCreateDescriptorSetLayout(m_MainDevice.device, &uboSkyboxSetLayoutCreateInfo, nullptr, &m_SkyboxDescriptorSetLayout[0]);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create description set layout\n");

	// Configure Cubemap descriptor set layout
	VkDescriptorSetLayoutBinding cubemapLayoutBinding = Vki::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1);
	VkDescriptorSetLayoutCreateInfo cubemapSetLayoutCreateInfo = Vki::DescriptorSetLayoutCreateInfo(cubemapLayoutBinding, 1);

	re = vkCreateDescriptorSetLayout(m_MainDevice.device, &cubemapSetLayoutCreateInfo, nullptr, &m_SkyboxDescriptorSetLayout[1]);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create description set layout\n");
	// ------------------------
}

void VulkanRenderer::AllocateDescriptorSets()
{
	m_GlobalDescriptorSet.resize(3);

	VkDescriptorSetAllocateInfo uboDescriptorSetAllocInfo = Vki::AllocateDescriptorSet(1, m_DescriptorPool, m_DescriptorSetLayout[0]);
	auto re = vkAllocateDescriptorSets(m_MainDevice.device, &uboDescriptorSetAllocInfo, &m_GlobalDescriptorSet[0]);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to allocate description set\n");

	VkDescriptorSetAllocateInfo samplerDescriptorSetAllocInfo = Vki::AllocateDescriptorSet(1, m_DescriptorPool, m_DescriptorSetLayout[1]);
	re = vkAllocateDescriptorSets(m_MainDevice.device, &samplerDescriptorSetAllocInfo, &m_GlobalDescriptorSet[1]);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to allocate description set\n");

	VkDescriptorSetAllocateInfo textureDescriptorSetAllocInfo = Vki::AllocateDescriptorSet(1, m_DescriptorPool, m_DescriptorSetLayout[2]);
	re = vkAllocateDescriptorSets(m_MainDevice.device, &textureDescriptorSetAllocInfo, &m_GlobalDescriptorSet[2]);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to allocate description set\n");

	// ----------TEMP----------
	m_SkyboxDescriptorSet.resize(2);

	VkDescriptorSetAllocateInfo uboSkyboxDescriptorSetAllocInfo = Vki::AllocateDescriptorSet(1, m_SkyboxDescriptorPool, m_SkyboxDescriptorSetLayout[0]);
	re = vkAllocateDescriptorSets(m_MainDevice.device, &uboSkyboxDescriptorSetAllocInfo, &m_SkyboxDescriptorSet[0]);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to allocate description set\n");

	VkDescriptorSetAllocateInfo cubemapDescriptorSetAllocInfo = Vki::AllocateDescriptorSet(1, m_SkyboxDescriptorPool, m_SkyboxDescriptorSetLayout[1]);
	re = vkAllocateDescriptorSets(m_MainDevice.device, &cubemapDescriptorSetAllocInfo, &m_SkyboxDescriptorSet[1]);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to allocate description set\n");
	// ------------------------
}

void VulkanRenderer::WriteDescriptors()
{
	std::array<VkWriteDescriptorSet, 5> descriptorWriter{};

	VkDescriptorBufferInfo descriptorBufferInfo{};
	descriptorBufferInfo.buffer = m_Camera.GetUniformBuffer().buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(CameraTransform);

	VkDescriptorImageInfo samplerInfo{};
	samplerInfo.sampler = m_Sampler;

	std::vector<VkDescriptorImageInfo> texturesInfo{};
	texturesInfo.resize(VulkanTexture::GetTextureDatabase().size());

	for (uint8_t i = 0; i < texturesInfo.size(); ++i)
	{
		texturesInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		texturesInfo[i].imageView = VulkanTexture::GetTextureDatabase()[i].imageView;
		texturesInfo[i].sampler = nullptr;
	}

	descriptorWriter[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriter[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWriter[0].dstSet = m_GlobalDescriptorSet[0];
	descriptorWriter[0].dstBinding = 0;
	descriptorWriter[0].dstArrayElement = 0;
	descriptorWriter[0].descriptorCount = 1;
	descriptorWriter[0].pBufferInfo = &descriptorBufferInfo;

	descriptorWriter[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriter[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	descriptorWriter[1].dstSet = m_GlobalDescriptorSet[1];
	descriptorWriter[1].dstBinding = 0;
	descriptorWriter[1].dstArrayElement = 0;
	descriptorWriter[1].descriptorCount = 1;
	descriptorWriter[1].pImageInfo = &samplerInfo;

	descriptorWriter[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriter[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	descriptorWriter[2].dstSet = m_GlobalDescriptorSet[2];
	descriptorWriter[2].dstBinding = 0;
	descriptorWriter[2].dstArrayElement = 0;
	descriptorWriter[2].descriptorCount = static_cast<uint32_t>(texturesInfo.size());
	descriptorWriter[2].pImageInfo = texturesInfo.data();

	descriptorWriter[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriter[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWriter[3].dstSet = m_SkyboxDescriptorSet[0];
	descriptorWriter[3].dstBinding = 0;
	descriptorWriter[3].dstArrayElement = 0;
	descriptorWriter[3].descriptorCount = 1;
	descriptorWriter[3].pBufferInfo = &descriptorBufferInfo;
	
	VkDescriptorImageInfo cubemapSamplerInfo{};
	cubemapSamplerInfo.sampler = m_Sampler;
	cubemapSamplerInfo.imageView = m_Skybox.GetCubemapTexture().GetTextureData().imageView;
	cubemapSamplerInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	descriptorWriter[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWriter[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWriter[4].dstSet = m_SkyboxDescriptorSet[1];
	descriptorWriter[4].dstBinding = 0;
	descriptorWriter[4].dstArrayElement = 0;
	descriptorWriter[4].descriptorCount = 1;
	descriptorWriter[4].pImageInfo = &cubemapSamplerInfo;

	vkUpdateDescriptorSets(m_MainDevice.device, static_cast<uint32_t>(descriptorWriter.size()), descriptorWriter.data(), 0, nullptr);
}

void VulkanRenderer::CreateTextureSampler()
{
	VkSamplerCreateInfo samplerCreateInfo{};

	VkBool32 anisotropySupport = m_MainDevice.physicalDeviceFeatures.samplerAnisotropy;

	if (anisotropySupport)
	{
		const float maxAnisotropy = m_MainDevice.physicalDeviceProperties.limits.maxSamplerAnisotropy;
		samplerCreateInfo = Vki::SamplerCreateInfo(VK_TRUE, maxAnisotropy);
	}
	else
	{
		samplerCreateInfo = Vki::SamplerCreateInfo(VK_FALSE);
	}

	VkResult re = vkCreateSampler(m_MainDevice.device, &samplerCreateInfo, nullptr, &m_Sampler);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create sampler\n");
}

void VulkanRenderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& _debugUtilsCreateInfo)
{
	_debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	_debugUtilsCreateInfo.flags = 0;
	_debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	_debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	_debugUtilsCreateInfo.pfnUserCallback = DebugCallback;
	_debugUtilsCreateInfo.pUserData = nullptr;
}

void VulkanRenderer::RecordCommands()
{
	VkCommandBufferBeginInfo bufferBeginInfo{};
	bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkRenderPassBeginInfo renderPassBeginInfo{};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = m_RenderPass;												// Render Pass to begin
	renderPassBeginInfo.renderArea.offset = { 0, 0 };											// Start point of render pass in pixels
	renderPassBeginInfo.renderArea.extent = m_Swapchain.GetSwapchainImageExtent();				// Size of region to run render pass on (starting at offset)

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.1f, 0.1f, 0.45f, 1.0f };
	clearValues[1].depthStencil.depth = 1.0f;

	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());		// Number of clear values
	renderPassBeginInfo.pClearValues = clearValues.data();									// List of clear values

	m_Camera.SetView(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	m_Camera.SetProjection(glm::radians(60.0f), (float)m_Swapchain.GetSwapchainImageExtent().width / m_Swapchain.GetSwapchainImageExtent().height, 0.1f, 100.0f);

	for (uint16_t i = 0; i < m_CommandBuffers.size(); ++i)
	{
		renderPassBeginInfo.framebuffer = m_Framebuffers[i];

		vkBeginCommandBuffer(m_CommandBuffers[i], &bufferBeginInfo);
		vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// -----------------------
		vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline2);
		
		vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout2, 0, 1, &m_SkyboxDescriptorSet[0], 0, nullptr);
		vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout2, 1, 1, &m_SkyboxDescriptorSet[1], 0, nullptr);

		m_Skybox.Bind(m_CommandBuffers[i]);
		m_Skybox.Render(m_CommandBuffers[i]);
		// -----------------------

		vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

		vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_GlobalDescriptorSet[0], 0, nullptr);
		vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 1, 1, &m_GlobalDescriptorSet[1], 0, nullptr);
		vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 2, 1, &m_GlobalDescriptorSet[2], 0, nullptr);

		for (auto& gameObject : m_GameObjects)
		{
			gameObject.UpdateModelMatrix();
			ObjectData objectData = gameObject.GetObjectData();
			vkCmdPushConstants(m_CommandBuffers[i], m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ObjectData), &objectData);

			gameObject.Bind(m_CommandBuffers[i]);
			gameObject.Render(m_CommandBuffers[i]);
		}

		vkCmdEndRenderPass(m_CommandBuffers[i]);
		vkEndCommandBuffer(m_CommandBuffers[i]);
	}
}

void VulkanRenderer::SetupScene()
{
	const std::vector<std::string> skyboxTextures =
	{
		"sb_right.jpg",
		"sb_left.jpg",
		"sb_top.jpg",
		"sb_bottom.jpg",
		"sb_front.jpg",
		"sb_back.jpg"
	};

	m_Skybox.LoadSkyboxTextures(skyboxTextures);

	GameObject ground(VulkanPrimative::Primative::Quad, "volcanic_rock_0.jpg");
	ground.SetPosition(glm::vec3(0.0f, 0.5f, 0.0f));
	ground.SetScale(glm::vec3(10.0f, 10.0f, 1.0f));
	ground.SetRotation(glm::vec3(1.57f, 0.0f, 0.0f));

	GameObject box1(VulkanPrimative::Primative::Cube, "brick_0.jpg");
	GameObject box2(VulkanPrimative::Primative::Cube, "brick_0.jpg");
	GameObject box3(VulkanPrimative::Primative::Cube, "brick_0.jpg");
	GameObject box4(VulkanPrimative::Primative::Cube, "brick_0.jpg");
	GameObject box5(VulkanPrimative::Primative::Cube, "brick_0.jpg");
	
	box2.SetPosition(glm::vec3(1.5f, 0.0f, 0.0f));
	box2.SetRotation(glm::vec3(0.0f, 1.1f, 0.0f));
	box3.SetPosition(glm::vec3(3.0f, 0.0f, 0.0f));
	box4.SetPosition(glm::vec3(-1.5f, 0.0f, 0.0f));
	box4.SetRotation(glm::vec3(0.0f, 1.1f, 0.0f));
	box5.SetPosition(glm::vec3(-3.0f, 0.0f, 0.0f));

	m_GameObjects.emplace_back(ground);
	m_GameObjects.emplace_back(box1);
	m_GameObjects.emplace_back(box2);
	m_GameObjects.emplace_back(box3);
	m_GameObjects.emplace_back(box4);
	m_GameObjects.emplace_back(box5);
}