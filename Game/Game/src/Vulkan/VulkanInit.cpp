#include "VulkanInit.h"

namespace Vki
{
    VkApplicationInfo Vki::AppInfo(const char* _appName, const uint32_t _appVersion, const char* _engineName, const uint32_t _engineVersion, const uint32_t _vulkanVersion)
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = _appName;
        appInfo.applicationVersion = _appVersion;
        appInfo.pEngineName = _engineName;
        appInfo.engineVersion = _engineVersion;
        appInfo.apiVersion = _vulkanVersion;
        return appInfo;
    }

    VkInstanceCreateInfo InstanceCreateInfo(const VkApplicationInfo& _appInfo, const std::vector<const char*>& _instanceExtensions, const std::vector<const char*>& _layersToEnable)
    {
        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &_appInfo;
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;

        if (_layersToEnable.size() > 0)
        {
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(_layersToEnable.size());
            instanceCreateInfo.ppEnabledLayerNames = _layersToEnable.data();
        }

        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(_instanceExtensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = _instanceExtensions.data();
        return instanceCreateInfo;
    }

    VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo(const VkInstance& _vkInstance)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        debugMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        return debugMessengerCreateInfo;
    }

    VkDeviceQueueCreateInfo QueueCreateInfo(const uint32_t _queueCount, const uint32_t _queueFamilyIndex)
    {
        VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.flags = 0;
        deviceQueueCreateInfo.queueCount = _queueCount;
        deviceQueueCreateInfo.queueFamilyIndex = _queueFamilyIndex;
        const float queuePriority = 1.0f;
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
        return deviceQueueCreateInfo;
    }

    VkDeviceCreateInfo DeviceCreateInfo(const VkPhysicalDeviceFeatures& _deviceFeatures, const std::vector<VkDeviceQueueCreateInfo>& _queueCreateInfo, const std::vector<const char*>& _deviceExtensions)
    {
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.flags = 0;
        deviceCreateInfo.pEnabledFeatures = &_deviceFeatures;
        deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(_queueCreateInfo.size());
        deviceCreateInfo.pQueueCreateInfos = _queueCreateInfo.data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames = _deviceExtensions.data();
        deviceCreateInfo.enabledLayerCount = 0;
        deviceCreateInfo.ppEnabledLayerNames = nullptr;
        return deviceCreateInfo;
    }

    VkSwapchainCreateInfoKHR SwapchainCreateInfo()
    {
        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.imageArrayLayers = 1;
        swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.clipped = VK_TRUE;
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;							
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
        swapchainCreateInfo.oldSwapchain = nullptr;
        return swapchainCreateInfo;
    }

    VkShaderModuleCreateInfo ShaderModuleCreateInfo(const std::vector<char>& _shaderCode)
    {
        VkShaderModuleCreateInfo shaderModuleCreateInfo{};
        shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.codeSize = _shaderCode.size();
        shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(_shaderCode.data());
        return shaderModuleCreateInfo;
    }

    VkPipelineShaderStageCreateInfo ShaderStageCreateInfo(const VkShaderStageFlagBits _shaderStage, const VkShaderModule& _shaderModule)
    {
        VkPipelineShaderStageCreateInfo vertexShaderCreateInfo{};
        vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderCreateInfo.stage = _shaderStage;
        vertexShaderCreateInfo.module = _shaderModule;
        vertexShaderCreateInfo.pName = "main";
        return vertexShaderCreateInfo;
    }

    VkVertexInputBindingDescription VertexInputBindingDescription(const uint32_t _binding, const uint32_t _stride, const VkVertexInputRate _inputRate)
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = _binding;
        bindingDescription.stride = _stride;
        bindingDescription.inputRate = _inputRate;
        return bindingDescription;
    }

    VkVertexInputAttributeDescription VertexInputAttributeDescription(const uint32_t _binding, const uint32_t _location, const VkFormat _format, const uint32_t _offset)
    {
        VkVertexInputAttributeDescription inputAttributeDescription{};
        inputAttributeDescription.binding = _binding;
        inputAttributeDescription.location = _location;
        inputAttributeDescription.format = _format;
        inputAttributeDescription.offset = _offset;
        return inputAttributeDescription;
    }

    VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo(const uint32_t _bindingCount, const VkVertexInputBindingDescription& _bindingDescription, const std::vector<VkVertexInputAttributeDescription>& _vertexAttributes)
    {
        VkPipelineVertexInputStateCreateInfo inputStateCreateInfo{};
        inputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        inputStateCreateInfo.vertexBindingDescriptionCount = _bindingCount;
        inputStateCreateInfo.pVertexBindingDescriptions = &_bindingDescription;
        inputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(_vertexAttributes.size());
        inputStateCreateInfo.pVertexAttributeDescriptions = _vertexAttributes.data();
        return inputStateCreateInfo;
    }

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo(const VkPrimitiveTopology _topology, const VkBool32 _restartPrimitive)
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = _topology;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = _restartPrimitive;
        return inputAssemblyStateCreateInfo;
    }

    VkPipelineViewportStateCreateInfo ViewportStateCreateInfo(const uint32_t _viewportCount, const VkViewport& _viewport, const uint32_t _scissorCount, const VkRect2D& _scissor)
    {
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = _viewportCount;
        viewportStateCreateInfo.pViewports = &_viewport;
        viewportStateCreateInfo.scissorCount = _scissorCount;
        viewportStateCreateInfo.pScissors = &_scissor;
        return viewportStateCreateInfo;
    }

    VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(const VkPolygonMode _polyMode, const VkCullModeFlagBits _cullMode, const VkFrontFace _frontFace)
    {
        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
        rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode = _polyMode;
        rasterizationStateCreateInfo.lineWidth = 1.0f;
        rasterizationStateCreateInfo.cullMode = _cullMode;
        rasterizationStateCreateInfo.frontFace = _frontFace;
        rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        return rasterizationStateCreateInfo;
    }

    VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo(const VkBool32 _enableSampling, const VkSampleCountFlagBits _samples)
    {
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.sampleShadingEnable = _enableSampling;
        multisampleStateCreateInfo.rasterizationSamples = _samples;
        return multisampleStateCreateInfo;
    }

    VkPipelineColorBlendAttachmentState ColorBlendAttachmentState(const VkBool32 _enableBlend, const VkColorComponentFlags _colorWriteMask)
    {
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
        colorBlendAttachmentState.blendEnable = _enableBlend;
        colorBlendAttachmentState.colorWriteMask = _colorWriteMask;
        return colorBlendAttachmentState;
    }

    VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo(const VkBool32 _logicOpEnable, const uint32_t _attachmentCount, const VkPipelineColorBlendAttachmentState& _attachment)
    {
        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.logicOpEnable = _logicOpEnable;
        colorBlendStateCreateInfo.attachmentCount = _attachmentCount;
        colorBlendStateCreateInfo.pAttachments = &_attachment;
        return colorBlendStateCreateInfo;
    }

    VkPipelineLayoutCreateInfo LayoutCreateInfo()
    {
        VkPipelineLayoutCreateInfo layoutCreateInfo{};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        return layoutCreateInfo;
    }

    VkPipelineDepthStencilStateCreateInfo DepthStencilStateCreateInfo(const VkBool32 _enableDepthTest, const VkBool32 _enableDepthWrite, const VkCompareOp _compareOp, const VkBool32 _enableDepthBoundsTest, const VkBool32 _enableStencilTest)
    {
        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.depthTestEnable = _enableDepthTest;                                                 // Enable checking depth to determine fragment write
        depthStencilStateCreateInfo.depthWriteEnable = _enableDepthWrite;                                               // Enable writing to depth buffer (to replace old values)
        depthStencilStateCreateInfo.depthCompareOp = _compareOp;                                                        // Comparison operation that allows an overwrite (is in front)
        depthStencilStateCreateInfo.depthBoundsTestEnable = _enableDepthBoundsTest;                                     // Depth bounds test: does the depth value exist between two bounds
        depthStencilStateCreateInfo.stencilTestEnable = _enableStencilTest;                                             // Enable stencil test
        return depthStencilStateCreateInfo;
    }

    VkViewport ViewportInfo(const VkExtent2D& _viewportSize)
    {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(_viewportSize.width);
        viewport.height = static_cast<float>(_viewportSize.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        return viewport;
    }

    VkRect2D ScissorInfo(const VkExtent2D& _scissorSize)
    {
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = _scissorSize;
        return scissor;
    }

    VkPushConstantRange PushConstantRange(const VkShaderStageFlags _shaderStage, const uint32_t _offset, const uint32_t _size)
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = _shaderStage;
        pushConstantRange.offset = _offset;
        pushConstantRange.size = _size;
        return pushConstantRange;
    }

    VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo()
    {
        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
        graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        graphicsPipelineCreateInfo.basePipelineIndex = -1;
        return graphicsPipelineCreateInfo;
    }

    VkImageCreateInfo ImageCreateInfo()
    {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        return imageCreateInfo;
    }

    VkImageViewCreateInfo ImageViewCreateInfo(const VkImage& _image, const VkFormat _format, const VkImageAspectFlags _aspectFlags)
    {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = _image;											// Image to create image view for
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;						// Type of image (1D, 2D, 3D, cubemap...)
        imageViewCreateInfo.format = _format;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // Subresources allow the view to view only a part of the image 
        imageViewCreateInfo.subresourceRange.aspectMask = _aspectFlags;				// Which aspect of the image to view (e.g. COLOR_BIT to view color)
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;						// Start mipmap level to view from
        imageViewCreateInfo.subresourceRange.levelCount = 1;						// Number of mipmap levels to view
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;					// Start array level to view from
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        return imageViewCreateInfo;
    }

    VkRenderPassCreateInfo RenderPassCreateInfo()
    {
        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        return renderPassCreateInfo;
    }

    VkFramebufferCreateInfo FramebufferCreateInfo(const VkExtent2D& _framebufferSize)
    {
        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.width = _framebufferSize.width;
        framebufferCreateInfo.height = _framebufferSize.height;
        framebufferCreateInfo.layers = 1;
        return framebufferCreateInfo;
    }

    VkSamplerCreateInfo SamplerCreateInfo(const VkBool32 _enableAnisotropy, const float _maxAnisotropy)
    {
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        if (_enableAnisotropy)
        {
            samplerCreateInfo.anisotropyEnable = VK_TRUE;
            samplerCreateInfo.maxAnisotropy = _maxAnisotropy;
        }
        else
        {
            samplerCreateInfo.anisotropyEnable = VK_FALSE;
            samplerCreateInfo.maxAnisotropy = 1.0f;
        }

        samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
        samplerCreateInfo.compareEnable = VK_FALSE;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 0.0f;

        return samplerCreateInfo;
    }

    VkCommandPoolCreateInfo CommandPoolCreateInfo(const uint32_t _queueFamilyIndex)
    {
        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex = _queueFamilyIndex;                 // Queue family type that buffers from this command pool will use
        return commandPoolCreateInfo;
    }

    VkCommandBufferAllocateInfo AllocateCommandBuffer(const VkCommandPool& _commandPool, const uint32_t _bufferCount, const VkCommandBufferLevel _level)
    {
        VkCommandBufferAllocateInfo bufferAllocInfo{};
        bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        bufferAllocInfo.commandPool = _commandPool;
        bufferAllocInfo.commandBufferCount = _bufferCount;
        bufferAllocInfo.level = _level;
        // VK_COMMAND_BUFFER_LEVEL_PRIMARY   : Buffer you submit directly to queue, can't be called by other buffers                           
        // VK_COMMAND_BUFFER_LEVEL_SECONDARY : Buffer can't be called directly, can be called from other buffers via "vkCmdExecuteCommands" when recording commands
        return bufferAllocInfo;
    }

    VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& _poolSizes, const uint32_t _maxSets)
    {
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.maxSets = _maxSets;
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(_poolSizes.size());
        descriptorPoolCreateInfo.pPoolSizes = _poolSizes.data();
        return descriptorPoolCreateInfo;
    }

    VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(const uint32_t _binding, const VkDescriptorType _descriptorType, const VkShaderStageFlags _shaderStage, const uint32_t _descriptorCount)
    {
        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
        descriptorSetLayoutBinding.binding = _binding;
        descriptorSetLayoutBinding.descriptorType = _descriptorType;
        descriptorSetLayoutBinding.stageFlags = _shaderStage;
        descriptorSetLayoutBinding.descriptorCount = _descriptorCount;
        descriptorSetLayoutBinding.pImmutableSamplers = nullptr;
        return descriptorSetLayoutBinding;
    }

    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutBinding& _layoutBinding, const uint32_t _bindingCount)
    {
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.bindingCount = _bindingCount;
        descriptorSetLayoutCreateInfo.pBindings = &_layoutBinding;
        return descriptorSetLayoutCreateInfo;
    }

    VkDescriptorSetAllocateInfo AllocateDescriptorSet(const uint32_t _setCount, const VkDescriptorPool& _pool, const VkDescriptorSetLayout& _setLayout)
    {
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorSetCount = _setCount;
        descriptorSetAllocInfo.pSetLayouts = &_setLayout;
        descriptorSetAllocInfo.descriptorPool = _pool;
        return descriptorSetAllocInfo;
    }

    VkBufferCreateInfo BufferCreateInfo(const VkBufferUsageFlags _usage, const VkDeviceSize _bufferSize)
    {
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.usage = _usage;
        bufferCreateInfo.size = _bufferSize;
        bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        return bufferCreateInfo;
    }

    VkMemoryAllocateInfo AllocateMemoryInfo(const VkDeviceSize _memorySize, const uint32_t _memoryTypeIndex)
    {
        VkMemoryAllocateInfo memoryAllocInfo{};
        memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocInfo.allocationSize = _memorySize;
        memoryAllocInfo.memoryTypeIndex = _memoryTypeIndex;
        return memoryAllocInfo;
    }
}