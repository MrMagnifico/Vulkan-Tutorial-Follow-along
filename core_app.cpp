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

/// <summary>
/// Temporary helper function, creates a 1x1x1 cube centered at offset
/// </summary>
/// <param name="device"></param>
/// <param name="offset"></param>
/// <returns></returns>
std::unique_ptr<Model> createCubeModel(LogicalDevice& device, glm::vec3 offset) {
    std::vector<Vertex> vertices{

        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

    };
    for (auto& v : vertices) {
        v.pos += offset;
    }
    return std::make_unique<Model>(device, vertices);
}

void
CoreApp::loadSceneObjects() {
	// Load model(s)
    std::shared_ptr<Model> cube_model_ptr = createCubeModel(vulkan_device, { 0.0f, 0.0f, 0.0f });

	// Generate object(s)
    TransformComponent transformations{};
    transformations.translation = { 0.0f, 0.0f, 0.0f };
    transformations.scale = { 0.5f, 0.5f, 0.5f };
    transformations.rotation = { 0.5f, 0.5f, 0.0f };
	SceneObject cube = SceneObject(cube_model_ptr, transformations);
	scene_objects.push_back(std::move(cube));
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
