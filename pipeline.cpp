#include "files.hpp"
#include "pipeline.hpp"

#include <stdexcept>

Pipeline::Pipeline(
	Device& device,
	const std::string& vert_file_path,
	const std::string& frag_file_path,
	const PipelineConfigInfo& config_info)
	: device{ device } {

	createGraphicsPipeline(vert_file_path, frag_file_path, config_info);
}

void Pipeline::createGraphicsPipeline(
	const std::string& vert_file_path,
	const std::string& frag_file_path,
	const PipelineConfigInfo& config_info) {

	if (config_info.pipeline_layout == VK_NULL_HANDLE) {
		throw std::runtime_error("Pipeline fixed function config info does not include a pipeline layout");
	}
	if (config_info.render_pass == VK_NULL_HANDLE) {
		throw std::runtime_error("Pipeline fixed function config info does not include a render pass");
	}

	// START OF PROGRAMMABLE STAGES CREATION
	std::vector<char> vert_shader_code = FileUtils::readFile("shaders/vert.spv");
	std::vector<char> frag_shader_code = FileUtils::readFile("shaders/frag.spv");
	VkShaderModule vert_shader_module = ShaderUtils::createShaderModule(device.getDevice(), vert_shader_code);
	VkShaderModule frag_shader_module = ShaderUtils::createShaderModule(device.getDevice(), frag_shader_code);

	VkPipelineShaderStageCreateInfo vert_shader_stage_create_info{};
	vert_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT; // Specify that this shader belongs to the vertex shader stage of the pipeline
	vert_shader_stage_create_info.module = vert_shader_module;
	vert_shader_stage_create_info.pName = "main";
	VkPipelineShaderStageCreateInfo frag_shader_stage_create_info{};
	frag_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT; // Specify that this shader belongs to the fragment shader stage of the pipeline
	frag_shader_stage_create_info.module = frag_shader_module;
	frag_shader_stage_create_info.pName = "main";
	VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_create_info, frag_shader_stage_create_info };
	// END OF PROGRAMMABLE STAGES CREATION

	// First stage of fixed function stages set
	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	
	// START OF PIPELINE CREATION
	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	// Shader/programmable stages data
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;

	// Fixed function stages data
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
	pipeline_info.pViewportState = &config_info.viewport_info;
	pipeline_info.pRasterizationState = &config_info.rasterization_info;
	pipeline_info.pMultisampleState = &config_info.multisample_info;
	pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
	pipeline_info.pColorBlendState = &config_info.color_blend_info;
	pipeline_info.pDynamicState = nullptr; // Optional

	pipeline_info.layout = config_info.pipeline_layout;

	// Defines which render pass this pipeline will belong to and which subpass it constitutes
	pipeline_info.renderPass = config_info.render_pass;
	pipeline_info.subpass = 0;

	// No parent pipeline to derive from (would also require a VK_PIPELINE_CREATE_DERIVATIVE_BIT flag)
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipeline_info.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create graphics pipeline");
	}
	// END OF PIPELINE CREATION

	vkDestroyShaderModule(device.getDevice(), vert_shader_module, nullptr);
	vkDestroyShaderModule(device.getDevice(), frag_shader_module, nullptr);
}

void
Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& config_info, uint32_t width, uint32_t height) {
	config_info.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	config_info.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Specifies that each 3 vertices describe a triangle with no reuse, see (https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions)
	config_info.input_assembly_info.primitiveRestartEnable = VK_FALSE; // Specifies whether or not a MAX value specifices the restart of assembly, see (https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions)

	// Describes region of the frame buffer to render to
	config_info.viewport.x = 0.0f;
	config_info.viewport.y = 0.0f;
	config_info.viewport.width = static_cast<float>(width); // We want to render to the entire extent of the swap chain image
	config_info.viewport.height = static_cast<float>(height); // Same here
	config_info.viewport.minDepth = 0.0f;
	config_info.viewport.maxDepth = 1.0f;

	// Defines region where pixels should actually be rendered (basically clipping)
	config_info.scissor.offset = { 0, 0 };
	config_info.scissor.extent = { width, height };

	config_info.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	config_info.viewport_info.viewportCount = 1;
	config_info.viewport_info.pViewports = &config_info.viewport;
	config_info.viewport_info.scissorCount = 1;
	config_info.viewport_info.pScissors = &config_info.scissor;

	config_info.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	config_info.rasterization_info.depthClampEnable = VK_FALSE; // Fragments outside the near and far planes are discarded instead of being clamped to the planes
	config_info.rasterization_info.rasterizerDiscardEnable = VK_FALSE; // Do not discard geometry being fed into the rasteriser (basically actually render stuff and don't just fuckin' yeet it)
	config_info.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL; // Fills area between lines of polygons with fragments
	config_info.rasterization_info.lineWidth = 1.0f; // Thickness of rendered lines in terms of fragments
	config_info.rasterization_info.cullMode = VK_CULL_MODE_BACK_BIT; // Back-facing triangles should be culled/discarded
	config_info.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE; // Specifies that a triangle with negative area is considered front-facing, see (https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkFrontFace.html)
	config_info.rasterization_info.depthBiasEnable = VK_FALSE; // Do not apply a bias to fragment depth values

	config_info.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	config_info.multisample_info.sampleShadingEnable = VK_FALSE;
	config_info.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Use only a single sample (i.e: no multi-sampling, jaggies galore)

	// See (https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions)
	config_info.color_blend_attachment.colorWriteMask =  // Final written colour should combine red, green, blue and alpha channels
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	config_info.color_blend_attachment.blendEnable = VK_FALSE;

	config_info.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	config_info.color_blend_info.logicOpEnable = VK_FALSE;
	config_info.color_blend_info.attachmentCount = 1;
	config_info.color_blend_info.pAttachments = &config_info.color_blend_attachment;

	// config_info.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	// config_info.depth_stencil_info.depthTestEnable = VK_TRUE;
	// config_info.depth_stencil_info.depthWriteEnable = VK_TRUE;
	// config_info.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
	// config_info.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
	// config_info.depth_stencil_info.stencilTestEnable = VK_FALSE;
}
