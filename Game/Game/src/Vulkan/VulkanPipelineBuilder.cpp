#include "VulkanPipelineBuilder.h"
#include "VulkanInit.h"
#include <stdexcept>

VulkanPipelineBuilder::VulkanPipelineBuilder() :
	shaderStages{},
	vertexInputStateCreateInfo{},
	inputAssemblyStateCreateInfo{},
	viewportStateCreateInfo{},
	rasterizationStateCreateInfo{},
	multisampleStateCreateInfo{},
	colorBlendStateCreateInfo{},
	depthStencilStateCreateInfo{},
	layout(VK_NULL_HANDLE)
{}

VkPipeline VulkanPipelineBuilder::Build(const VkRenderPass& _renderPass, const VkDevice& _logicalDevice)
{
	VkPipeline newPipeline = VK_NULL_HANDLE;

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = Vki::GraphicsPipelineCreateInfo();
	graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	graphicsPipelineCreateInfo.pStages = shaderStages.data();
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = nullptr;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.layout = layout;
	graphicsPipelineCreateInfo.renderPass = _renderPass;
	graphicsPipelineCreateInfo.subpass = 0;

	VkResult re = vkCreateGraphicsPipelines(_logicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &newPipeline);
	if (re != VK_SUCCESS) throw std::runtime_error("VULKAN ERROR: Failed to create graphics pipeline\n");

	return newPipeline;
}
