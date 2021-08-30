#include "debug.hpp"
#include "device.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>

LogicalDevice::LogicalDevice(Window& window) : window{ window } {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createCommandPool();
}

LogicalDevice::~LogicalDevice() {
	vkDestroyCommandPool(device_, command_pool, nullptr);
	vkDestroyDevice(device_, nullptr);
	vkDestroySurfaceKHR(instance, surface_, nullptr);
	if (enable_validation_layers) DebugUtils::destroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void
LogicalDevice::createInstance() {
	if (enable_validation_layers && !checkValidationLayerSupport()) {
		throw std::runtime_error("Validation layer support needed but not found");
	}

	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Toy Application";
	app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	app_info.pEngineName = "Monke Engine 9000";
	app_info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	// Get needed extension data
	auto extensions = getRequiredExtensions();
	create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	create_info.ppEnabledExtensionNames = extensions.data();

	// Validation layers info
	VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
	if (enable_validation_layers) {
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();

		// Create messenger for instance creation and destruction debugging
		DebugUtils::populateDebugMessengerCreateInfo(debug_create_info);
		create_info.pNext = &debug_create_info;
	}
	else { create_info.enabledLayerCount = 0; }

	if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan instance");
	}
}

void
LogicalDevice::setupDebugMessenger() {
	if (!enable_validation_layers) return;

	VkDebugUtilsMessengerCreateInfoEXT create_info{};
	DebugUtils::populateDebugMessengerCreateInfo(create_info);

	if (DebugUtils::createDebugUtilsMessengerEXT(
		instance,
		&create_info,
		nullptr,
		&debug_messenger) != VK_SUCCESS) {
		throw std::runtime_error("Failed to set up debug messenger");
	}
}

void
LogicalDevice::createSurface() { window.createWindowSurface(instance, &surface_); }

void
LogicalDevice::pickPhysicalDevice() {
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
	if (device_count == 0) throw std::runtime_error("Failed to find GPUs with Vulkan support");

	std::vector<VkPhysicalDevice> devices(device_count);
	vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

	physical_device = findSuitableDevice(devices.data(), devices.size());
	if (physical_device == VK_NULL_HANDLE) throw std::runtime_error("Failed to find a suitable GPU");
}

void 
LogicalDevice::createLogicalDevice() {
	QueueFamilyIndices indices = findQueueFamilies(physical_device);
	float queue_priority = 1.0f;

	// Create one queue per queue family type
	std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
	std::set<uint32_t> unique_family_indices = { indices.graphics_family.value(), indices.present_family.value() };
	for (uint32_t family_index : unique_family_indices) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = family_index;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(queueCreateInfo);
	}

	// No particular device features are needed
	VkPhysicalDeviceFeatures device_features{};

	// Specify properties for logical device creation
	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queue_create_infos.data();
	create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	create_info.pEnabledFeatures = &device_features;
	create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
	create_info.ppEnabledExtensionNames = device_extensions.data();
	if (enable_validation_layers) {
		create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
		create_info.ppEnabledLayerNames = validation_layers.data();
	}
	else { create_info.enabledLayerCount = 0; }

	if (vkCreateDevice(physical_device, &create_info, nullptr, &device_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create logical device");
	}

	// Populate handles for the graphics and present queues
	vkGetDeviceQueue(device_, indices.graphics_family.value(), 0, &graphics_queue_);
	vkGetDeviceQueue(device_, indices.present_family.value(), 0, &present_queue_);
}

void
LogicalDevice::createCommandPool() {
	QueueFamilyIndices indices = findQueueFamilies(physical_device);

	VkCommandPoolCreateInfo command_pool_create_info = {};
	command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_create_info.queueFamilyIndex = indices.graphics_family.value();

	if (vkCreateCommandPool(device_, &command_pool_create_info, nullptr, &command_pool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool");
	}
}

bool
LogicalDevice::checkValidationLayerSupport() {
	uint32_t layer_count;
	vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	std::vector<VkLayerProperties> available_layers(layer_count);
	vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

	for (const char* needed_layer : validation_layers) {
		bool layer_found = false;

		for (const VkLayerProperties& existing_layer : available_layers) {
			if (strcmp(needed_layer, existing_layer.layerName) == 0) {
				layer_found = true;
				break;
			}
		}

		if (!layer_found) return false;
	}
	return true;
}

std::vector<const char*>
LogicalDevice::getRequiredExtensions() {
	// Extensions required by GLFW
	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

	// Validation layers debug utils extension
	if (enable_validation_layers) { extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

	return extensions;
}

VkPhysicalDevice
LogicalDevice::findSuitableDevice(VkPhysicalDevice* devices, size_t device_count) {
	for (uint32_t counter = 0; counter < device_count; counter++) {
		if (LogicalDevice::isDeviceSuitable(devices[counter], surface_)) return devices[counter];
	}
	return VK_NULL_HANDLE;
}

QueueFamilyIndices
LogicalDevice::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

	// Find first queue families supporting needed functions
	uint32_t i = 0;
	for (auto& queue_family : queue_families) {
		if (indices.isComplete()) break;

		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphics_family = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
		if (presentSupport) indices.present_family = i;

		i++;
	}

	return indices;
}

SwapChainSupportDetails
LogicalDevice::querySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	// Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

	// Surface formats (pixel format, color space)
	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, nullptr);
	if (format_count > 0) {
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &format_count, details.formats.data());
	}

	// Available presentation modes
	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_count, nullptr);
	if (present_mode_count > 0) {
		details.present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &present_mode_count, details.present_modes.data());
	}

	return details;
}

bool
LogicalDevice::isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR& surface) {
	// Graphics and present queue families exist
	QueueFamilyIndices indices = LogicalDevice::findQueueFamilies(device);

	// Required device extensions exist
	bool extensions_supported = LogicalDevice::checkDeviceExtensionSupport(device);

	// At least a single image format and a single presentation mode exist
	bool swap_chain_adequate = false;
	if (extensions_supported) {
		SwapChainSupportDetails swap_chain_support = LogicalDevice::querySwapChainSupport(device);
		swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
	}

	return indices.isComplete() && extensions_supported && swap_chain_adequate;
}

bool
LogicalDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	// Acquire extensions supported by device
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

	// Ensure needed extensions are supported
	std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());
	for (auto& extension : extensions) { required_extensions.erase(extension.extensionName); }
	return required_extensions.empty();
}
