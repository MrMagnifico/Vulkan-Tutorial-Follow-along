#pragma once

#include "device.hpp"
#include "pipeline.hpp"
#include "scene_object.hpp"

#include <memory>
#include <vector>

/// <summary>
/// Simple render system that simply allows for adding simple vertex and push constant data for a set of models
/// </summary>
class SimpleRenderSystem {
public:

	/// <summary>
	/// Create a new render system tied to the given device and render pass
	/// </summary>
	/// <param name="device">Device to tie rendering to</param>
	/// <param name="render_pass">Render pass to tie the rendering work to</param>
	SimpleRenderSystem(LogicalDevice& device, VkRenderPass render_pass);
	~SimpleRenderSystem();

	/// <summary>
	/// Bind pipeline and add push constant and model data to the given buffer
	/// </summary>
	/// <param name="command_buffer">Command buffer to add scene object render data to</param>
	void renderSceneObjects(VkCommandBuffer command_buffer, std::vector<SceneObject>& scene_objects);

private:
	LogicalDevice &vulkan_device;

	std::unique_ptr<GraphicsPipeline> pipeline;
	VkPipelineLayout pipeline_layout;

	void createPipelineLayout();
	void createPipeline(VkRenderPass render_pass);
};
