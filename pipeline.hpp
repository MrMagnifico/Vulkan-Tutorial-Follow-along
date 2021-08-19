#pragma once

#include "device.hpp"

#include <string>
#include <vector>

struct PipelineConfigInfo {
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo viewport_info;
	VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
	VkPipelineRasterizationStateCreateInfo rasterization_info;
	VkPipelineMultisampleStateCreateInfo multisample_info;
	VkPipelineColorBlendAttachmentState color_blend_attachment;
	VkPipelineColorBlendStateCreateInfo color_blend_info;
	VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
	VkPipelineLayout pipeline_layout = nullptr;
	VkRenderPass render_pass = nullptr;
	uint32_t subpass = 0;
};

class Pipeline {
public:
	Pipeline(
		Device& device,
		const std::string& vert_file_path,
		const std::string& frag_file_path,
		const PipelineConfigInfo& config_info);
	~Pipeline() { vkDestroyPipeline(device.getDevice(), pipeline, nullptr); }

	static void defaultPipelineConfigInfo(PipelineConfigInfo& config_info, uint32_t width, uint32_t height);

private:
	void createGraphicsPipeline(
		const std::string& vert_file_path,
		const std::string& frag_file_path,
		const PipelineConfigInfo& config_info);

	Device& device;
	VkPipeline pipeline;
};
