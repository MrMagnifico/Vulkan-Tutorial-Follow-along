#pragma once

#include "device.hpp"
#include "renderer.hpp"
#include "scene_object.hpp"
#include "window.hpp"

#include <memory>

/// <summary>
/// Entryway to application and manager of core resources (window - ubstabce abd devuce - renderer)
/// </summary>
class CoreApp {
public:
	static constexpr int WIDTH = 640;
	static constexpr int HEIGHT = 480;
	static constexpr float VERTICAL_FOV_DEG = 59.0;

	CoreApp();
	~CoreApp();

	/// <summary>
	/// Entrypoint to the application
	/// </summary>
	void run();

	/// <summary>
	/// Print supported instance extensions to stdout
	/// </summary>
	void printSupportedExtensions();

private:
	Window window{ WIDTH, HEIGHT, "Vulkan Tutorial" };
	LogicalDevice vulkan_device{ window };
	Renderer renderer{ window, vulkan_device };

	std::vector<SceneObject> scene_objects;

	void loadSceneObjects();
};
