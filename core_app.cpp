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
}

CoreApp::~CoreApp() {
	vkDestroyPipelineLayout(vulkan_device.getDevice(), pipeline_layout, nullptr);
}

void
CoreApp::run() {
	while (!window.shouldClose()) {
		glfwPollEvents();
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
	Pipeline::defaultPipelineConfigInfo(pipeline_config, device_swap_chain.getWidth(), device_swap_chain.getHeight());
	pipeline_config.render_pass = device_swap_chain.getRenderPass();
	pipeline_config.pipeline_layout = pipeline_layout;
	pipeline = std::make_unique<Pipeline>(
		vulkan_device,
		"shaders/vert.spv",
		"shaders/frag.spv",
		pipeline_config);
}

void
CoreApp::printSupportedExtensions() {
	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

	std::cout << "Found " << extension_count << " extensions\n";
	for (const VkExtensionProperties& extension : extensions) {
		std::cout << "\t" << extension.extensionName << "\n";
	}
}
