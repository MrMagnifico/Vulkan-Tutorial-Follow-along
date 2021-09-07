#include "core_app.hpp"
#include "simple_render_system.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <set>

CoreApp::CoreApp() {
	loadSceneObjects();
}

CoreApp::~CoreApp() {}

void
CoreApp::run() {
	SimpleRenderSystem simple_render_system(vulkan_device, renderer.getSwapChainRenderPass());

	while (!window.shouldClose()) {
		glfwPollEvents();
		
		if (VkCommandBuffer command_buffer = renderer.beginFrame()) {
			renderer.beginSwapChainRenderPass(command_buffer);
			simple_render_system.renderSceneObjects(command_buffer, scene_objects);
			renderer.endSwapChainRenderPass(command_buffer);
			renderer.endFrame();
		}
	}

	vkDeviceWaitIdle(vulkan_device.getDevice()); // Wait until all ongoing commands have ended before terminating
}

void
CoreApp::loadSceneObjects() {
	// Load model(s)
	std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
	};
	std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};
	auto model = std::make_shared<Model>(vulkan_device, vertices, indices);

	// Generate object(s)
	auto square = SceneObject(model);
	scene_objects.push_back(std::move(square));
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
