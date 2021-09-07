#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <set>

/// <summary>
/// Defines data to be provided as a push constant, taking into account alignment constraints.
/// See (https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/chap15.html#interfaces-resources-layout
/// and https://vulkan-tutorial.com/Uniform_buffers/Descriptor_pool_and_sets#page_Alignment-requirements)
/// </summary>
struct PushConstantData {
	alignas(16) glm::mat4 transformation;
};

SimpleRenderSystem::SimpleRenderSystem(LogicalDevice& device, VkRenderPass render_pass) : vulkan_device{ device } {
	createPipelineLayout();
	createPipeline(render_pass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
	vkDestroyPipelineLayout(vulkan_device.getDevice(), pipeline_layout, nullptr);
}

void
SimpleRenderSystem::renderSceneObjects(VkCommandBuffer command_buffer, std::vector<SceneObject>& scene_objects) {
	pipeline->bind(command_buffer);

	for (SceneObject object : scene_objects) {
		PushConstantData push_constant_data{};
		push_constant_data.transformation = object.transformation.affineMatrix();
		vkCmdPushConstants(
			command_buffer,
			pipeline_layout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(PushConstantData),
			&push_constant_data);

		object.model->bind(command_buffer);
		object.model->draw(command_buffer);
	}
}

void
SimpleRenderSystem::createPipelineLayout() {
	// Defines section of 'range' of memory that push constant data occupies
	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(PushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &push_constant_range;

	if (vkCreatePipelineLayout(vulkan_device.getDevice(), &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout");
	}
}

void
SimpleRenderSystem::createPipeline(VkRenderPass render_pass) {
	assert(pipeline_layout != nullptr && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipeline_config{};
	GraphicsPipeline::defaultPipelineConfigInfo(pipeline_config);
	pipeline_config.render_pass = render_pass;
	pipeline_config.pipeline_layout = pipeline_layout;
	pipeline = std::make_unique<GraphicsPipeline>(
		vulkan_device,
		"shaders/vert.spv",
		"shaders/frag.spv",
		pipeline_config);
}
