#include "files.hpp"

std::vector<char>
FileUtils::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary); // Start at end of stream and treat as binary data
	if (!file.is_open()) throw std::runtime_error("Failed to open file " + filename);

	size_t buffer_size = file.tellg();
	std::vector<char> buffer(buffer_size);

	file.seekg(0);
	file.read(buffer.data(), buffer_size);
	file.close();
	return buffer;
}

VkShaderModule
ShaderUtils::createShaderModule(const VkDevice& device, const std::vector<char>& code) {
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shader_module;
	if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module");
	}
	return shader_module;
}
