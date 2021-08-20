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

/// <summary>
/// Class for creating and managing a Vulkan graphics pipeline
/// </summary>
class GraphicsPipeline {
public:
	/// <summary>
	/// Creates a GraphicsPipeline object
	/// </summary>
	/// <param name="device">Device from which to derive the pipeline</param>
	/// <param name="vert_file_path">Path to a SPIR-V vertex shader file</param>
	/// <param name="frag_file_path">Path to a SPIR-V fragment shader file</param>
	/// <param name="config_info">Configuration information to be utilised in pipeline construction</param>
	GraphicsPipeline(
		LogicalDevice& device,
		const std::string& vert_file_path,
		const std::string& frag_file_path,
		const PipelineConfigInfo& config_info);
	~GraphicsPipeline() { vkDestroyPipeline(device.getDevice(), internal_pipeline, nullptr); }

	/// <summary>
	/// Initialises a pipeline config struct with preset default values
	/// </summary>
	/// <param name="config_info">A memory-allocated struct to initialise values within</param>
	/// <param name="width">Width of images to be rendered by the pipeline</param>
	/// <param name="height">Hiehgt of images to be rendered by the pipeline</param>
	static void defaultPipelineConfigInfo(PipelineConfigInfo& config_info, uint32_t width, uint32_t height);

private:
	/// <summary>
	/// Performs the actual creation of a graphics pipeline
	/// </summary>
	/// <param name="vert_file_path">Path to a SPIR-V vertex shader file</param>
	/// <param name="frag_file_path">Path to a SPIR-V fragment shader file</param>
	/// <param name="config_info">Configuration information to be utilised in pipeline construction</param>
	void createGraphicsPipeline(
		const std::string& vert_file_path,
		const std::string& frag_file_path,
		const PipelineConfigInfo& config_info);

	LogicalDevice& device;
	VkPipeline internal_pipeline;
};
