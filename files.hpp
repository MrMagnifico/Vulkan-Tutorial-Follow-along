#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <fstream>
#include <vector>
#include <string>

/// <summary>
/// Utility class containing static methods for manipulating files
/// </summary>
class FileUtils {
public:
	/// <summary>
	/// Reads a file, treating its contents as binary data
	/// </summary>
	/// <param name="filename">Name/Path to the file</param>
	/// <returns>A byte vector containing its data</returns>
	static std::vector<char> readFile(const std::string& filename);
};

/// <summary>
/// Utility class containing static methods for creating and manipulating shaders
/// </summary>
class ShaderUtils {
public:
	/// <summary>
	/// Create a Vulkan shader representation from raw SPIR-V data
	/// </summary>
	/// <param name="device">Logical device that creates the shader module</param>
	/// <param name="code">Raw binary data describing a SPIR-V shader</param>
	/// <returns>A Vulkan shader module wrapping the given shader code</returns>
	static VkShaderModule createShaderModule(VkDevice& device, const std::vector<char>& code);
};
