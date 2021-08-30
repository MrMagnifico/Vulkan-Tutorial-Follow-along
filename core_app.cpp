#include "core_app.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <set>

CoreApp::CoreApp() {
	createPipelineLayout();
	createPipeline();
	createCommandBuffers();
}

CoreApp::~CoreApp() {
	vkDestroyPipelineLayout(vulkan_device.getDevice(), pipeline_layout, nullptr);
}

void
CoreApp::run() {
	while (!window.shouldClose()) {
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(vulkan_device.getDevice()); // Wait until all ongoing commands have ended before terminating
}

void
CoreApp::drawFrame() {
	uint32_t image_index;
	auto result = device_swap_chain.acquireNextImage(&image_index);
	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { // Check that either image (with either full matching to surface specifications or not)
		throw std::runtime_error("Failed to acquire swapchain image");
	}

	result = device_swap_chain.submitCommandBuffers(&command_buffers[image_index], &image_index);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swapchain image");
	}
}

void
CoreApp::createPipelineLayout() {
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(vulkan_device.getDevice(), &pipelineLayoutInfo, nullptr, &pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create pipeline layout");
	}
}

void
CoreApp::createPipeline() {
	PipelineConfigInfo pipeline_config{};
	GraphicsPipeline::defaultPipelineConfigInfo(pipeline_config, device_swap_chain.getWidth(), device_swap_chain.getHeight());
	pipeline_config.render_pass = device_swap_chain.getRenderPass();
	pipeline_config.pipeline_layout = pipeline_layout;
	pipeline = std::make_unique<GraphicsPipeline>(
		vulkan_device,
		"shaders/vert.spv",
		"shaders/frag.spv",
		pipeline_config);
}

void
CoreApp::createCommandBuffers() {
	command_buffers.resize(device_swap_chain.imageCount()); // One command buffer per framebuffer (and we use one framebuffer per image)

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = vulkan_device.getCommandPool();
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t) command_buffers.size();
	if (vkAllocateCommandBuffers(vulkan_device.getDevice(), &alloc_info, command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed allocate command buffers");
	}

	for (size_t i = 0; i < command_buffers.size(); i++) {
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(command_buffers[i], &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer");
		}

		VkRenderPassBeginInfo render_pass_begin_info{};
		render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_begin_info.renderPass = device_swap_chain.getRenderPass();
		render_pass_begin_info.framebuffer = device_swap_chain.getFramebuffer(i);
		render_pass_begin_info.renderArea.offset = { 0, 0 };
		render_pass_begin_info.renderArea.extent = device_swap_chain.getSwapChainExtent();
		VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f }; // When clearing previous pixels, set their values to completely black with no transparency
		render_pass_begin_info.clearValueCount = 1;
		render_pass_begin_info.pClearValues = &clear_color;
		vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE); // Finalise render pass begin command

		pipeline->bind(command_buffers[i]);

		vkCmdDraw(
			command_buffers[i],
			3,	// We're only drawing 3 pre-defined vertices at the moment
			1,	// Instance count is set to 1 if instanced rendering is not being used
			0,	// Offset into vertex buffer, we start indexing from 0 as normal
			0);	// We don't use instanced rendering, so we keep this to 0 

		vkCmdEndRenderPass(command_buffers[i]);

		if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer");
		}
	}
}

void
CoreApp::printSupportedExtensions() {
	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

	std::cout << "Found " << extension_count << " instance extensions\n";
	for (const VkExtensionProperties& extension : extensions) {
		std::cout << "\t" << extension.extensionName << "\n";
	}
}
