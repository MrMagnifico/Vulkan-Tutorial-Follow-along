#include "debug.hpp"
#include "device.hpp"
#include "files.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <set>

const uint32_t WIDTH = 640;
const uint32_t HEIGHT = 480;
const char* TITLE = "Vulkan Test";

const std::vector<const char*> validation_layers = {
	"VK_LAYER_KHRONOS_validation"
};
#ifdef _DEBUG
const bool enable_validation_layers = true;
#else
const bool enable_validation_layers = false;
#endif


class HelloTriangleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

	void printSupportedExtensions() {
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

		std::cout << "Found " << extension_count << " extensions\n";
		for (const VkExtensionProperties& extension : extensions) {
			std::cout << "\t" << extension.extensionName << "\n";
		}
	}

private:
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debug_messenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;
	VkSwapchainKHR swap_chain;
	std::vector<VkImage> swap_chain_images;
	std::vector<VkImageView> swap_chain_image_views;
	VkPipelineLayout pipeline_layout;
	VkRenderPass render_pass;
	VkPipeline graphics_pipeline;

	void initWindow() {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Do not create OpenGL context
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Do not make window resizable

		window = glfwCreateWindow(WIDTH, HEIGHT, TITLE, nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createRenderPass();
		createGraphicsPipeline();
	}

	void mainLoop() {
		// GLFW loop
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		// Vulkan
		vkDestroyPipeline(device, graphics_pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
		vkDestroyRenderPass(device, render_pass, nullptr);
		for (auto& image_view : swap_chain_image_views) { vkDestroyImageView(device, image_view, nullptr); }
		vkDestroySwapchainKHR(device, swap_chain, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		if (enable_validation_layers) DebugUtils::destroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
		vkDestroyInstance(instance, nullptr);

		// GLFW
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void createInstance() {
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

	void setupDebugMessenger() {
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

	void createSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface");
		}
	}

	std::vector<const char*> getRequiredExtensions() {
		// Extensions required by GLFW
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

		// Validation layers debug utils extension
		if (enable_validation_layers) { extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); }

		return extensions;
	}

	void pickPhysicalDevice() {
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
		if (device_count == 0) throw std::runtime_error("Failed to find GPUs with Vulkan support");

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

		physical_device = DeviceUtils::findSuitableDevice(devices.data(), devices.size(), surface);
		if (physical_device == VK_NULL_HANDLE) throw std::runtime_error("Failed to find a suitable GPU");
	}

	void createLogicalDevice() {
		QueueFamilyIndices indices = DeviceUtils::findQueueFamilies(physical_device, surface);
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

		if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device");
		}

		// Populate handles for the graphics and present queues
		vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphics_queue);
		vkGetDeviceQueue(device, indices.present_family.value(), 0, &present_queue);
	}

	void createSwapChain() {
		SwapChainSupportDetails swap_chain_support = DeviceUtils::querySwapChainSupport(physical_device, surface);

		// Acquire swap chain initialisation details
		VkSurfaceFormatKHR surface_format = DeviceUtils::chooseSwapSurfaceFormat(swap_chain_support.formats);
		VkPresentModeKHR present_mode = DeviceUtils::chooseSwapPresentMode(swap_chain_support.present_modes);
		VkExtent2D extent = DeviceUtils::chooseSwapExtent(window, swap_chain_support.capabilities);

		// Choose appropriate minimum number of images to keep in swap chain at any time
		uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
		if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) {
			image_count = swap_chain_support.capabilities.maxImageCount;
		}

		// Populate swapchain creation struct
		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = surface;
		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Images will be used for direct colour attachment (i.e: presented as is with no further processing)
		create_info.preTransform = swap_chain_support.capabilities.currentTransform; // No transformation desired
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // Ignore alpha channel, no blending needed
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE; // Ignore obscured pixels (it's 2D occlussion culling bro)
		create_info.oldSwapchain = VK_NULL_HANDLE; // Only single swapchain for now, no fallback

		// Populate info about image sharing amongst queue families
		QueueFamilyIndices indices = DeviceUtils::findQueueFamilies(physical_device, surface);
		uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

		// Graphics and present family are different, sharing needed
		if (indices.graphics_family != indices.present_family) {
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		}
		// Graphics and present family are the same, set exclusive ownership as no sharing will be bothered with
		else {
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			create_info.queueFamilyIndexCount = 0;
			create_info.pQueueFamilyIndices = nullptr;
		}

		swap_chain_image_format = surface_format.format; // Store handle to image format
		swap_chain_extent = extent; // Store handle to surface extent
		if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swap_chain) != VK_SUCCESS) { // Create handle for actual swap chain
			throw std::runtime_error("Failed to create swapchain");
		}

		// Create handle for images in swap chain
		vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr);
		swap_chain_images.resize(image_count);
		vkGetSwapchainImagesKHR(device, swap_chain, &image_count, swap_chain_images.data());
	}

	void createImageViews() {
		swap_chain_image_views.resize(swap_chain_images.size());

		for (size_t i = 0; i < swap_chain_images.size(); i++) {
			VkImageViewCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.image = swap_chain_images[i];
			create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // Swap chain images are normal 2D images
			create_info.format = swap_chain_image_format; // Format is same as that of the images themselves

			// Colour components are kept as is (i.e: multiplied by an identity matrix)
			create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			// Swap chain images are single flat 2D images with a single mipmap level and a single layer
			create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Levels should be interpreted as straight colours
			create_info.subresourceRange.baseMipLevel = 0;
			create_info.subresourceRange.levelCount = 1;
			create_info.subresourceRange.baseArrayLayer = 0;
			create_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device, &create_info, nullptr, &swap_chain_image_views[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create image view(s)");
			}
		}
	}

	void createRenderPass() {
		VkAttachmentDescription colour_attachment{};
		colour_attachment.format = swap_chain_image_format;
		colour_attachment.samples = VK_SAMPLE_COUNT_1_BIT; // Should match multi-sampling sample count
		colour_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear colour and depth data before drawing new frame
		colour_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Save rendered data in memory (so we can actually display the goddamn thing)
		colour_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // Not doing anything with stencil buffer data, so we don't care
		colour_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colour_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // We don't care about previous image data
		colour_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Newly rendered data should be stored in a format suitable for presentation to the swapchain

		VkAttachmentReference color_attachment_ref{};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // Specify that this is a graphics subpass (because compute shaders are a thing, apparently)
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colour_attachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create render pass");
		}
	}

	void createGraphicsPipeline() {
		// START OF PROGRAMMABLE STAGES CREATION
		std::vector<char> vert_shader_code = FileUtils::readFile("shaders/vert.spv");
		std::vector<char> frag_shader_code = FileUtils::readFile("shaders/frag.spv");
		VkShaderModule vert_shader_module = ShaderUtils::createShaderModule(device, vert_shader_code);
		VkShaderModule frag_shader_module = ShaderUtils::createShaderModule(device, frag_shader_code);

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


		// START OF FIXED FUNCTION STAGES CREATION
		VkPipelineVertexInputStateCreateInfo vertex_input_info{};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		VkPipelineInputAssemblyStateCreateInfo input_assembly{};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Specifies that each 3 vertices describe a triangle with no reuse, see (https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions)
		input_assembly.primitiveRestartEnable = VK_FALSE; // Specifies whether or not a MAX value specifices the restart of assembly, see (https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions)

		// Describes region of the frame buffer to render to
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swap_chain_extent.width; // We want to render to the entire extent of the swap chain image
		viewport.height = (float)swap_chain_extent.height; // Same here
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		// Defines region where pixels should actually be render (basically clipping)
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = swap_chain_extent;

		VkPipelineViewportStateCreateInfo view_port_state_create_info{};
		view_port_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		view_port_state_create_info.viewportCount = 1;
		view_port_state_create_info.pViewports = &viewport;
		view_port_state_create_info.scissorCount = 1;
		view_port_state_create_info.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE; // Fragments outside the near and far planes are discarded instead of being clamped to the planes
		rasterizer.rasterizerDiscardEnable = VK_FALSE; // Do not discard geometry being fed into the rasteriser (basically actually render stuff and don't just fuckin' yeet it)
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // Fills area between lines of polygons with fragments
		rasterizer.lineWidth = 1.0f; // Thickness of rendered lines in terms of fragments
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // Back-facing traingles should be culled/discarded
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // Specifies that a triangle with negative area is considered front-facing, see (https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkFrontFace.html)
		rasterizer.depthBiasEnable = VK_FALSE; // Do not apply a bias to fragment depth values

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Use only a single sample (i.e: no multi-sampling, jaggies galore)

		// See (https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Fixed_functions)
		VkPipelineColorBlendAttachmentState color_blend_attachment{};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; // Final written colour should combine red, green, blue and alpha channels
		color_blend_attachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo color_blending{};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &color_blend_attachment;

		// Allows for adjustment of uniform/global variables in shaders
		VkPipelineLayoutCreateInfo pipeline_layout_info{};
		pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create pipeline layout");
		}
		// END OF FIXED FUNCTION STAGES CREATION

		// START OF PIPELINE CREATION
		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		
		// Shader/programmable stages data
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages;
		
		// Fixed function stages data
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pViewportState = &view_port_state_create_info;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pDepthStencilState = nullptr;
		pipeline_info.pColorBlendState = &color_blending;
		pipeline_info.pDynamicState = nullptr;

		pipeline_info.layout = pipeline_layout;

		// Defines which render pass this pipeline will belong to and which subpass it constitutes
		pipeline_info.renderPass = render_pass;
		pipeline_info.subpass = 0;

		// No parent pipeline to derive from (would also require a VK_PIPELINE_CREATE_DERIVATIVE_BIT flag)
		pipeline_info.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipeline_info.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create graphics pipeline");
		}
		// END OF PIPELINE CREATION

		vkDestroyShaderModule(device, vert_shader_module, nullptr);
		vkDestroyShaderModule(device, frag_shader_module, nullptr);
	}

	bool checkValidationLayerSupport() {
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
};

int main() {
	HelloTriangleApplication app;

	app.printSupportedExtensions();

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}