#pragma once

#include "device.hpp"

#include <glm/glm.hpp>

#include <array>

/// <summary>
/// Defines data elements of a vertex and methods to get Vulkan descriptions of these elements
/// </summary>
struct Vertex {
    /// <summary>
    /// Position of the vertex
    /// </summary>
    glm::vec2 pos;
    /// <summary>
    /// Color of the vertex
    /// </summary>
    glm::vec3 color;

    /// <summary>
    /// Creates a description of how to interpret vertex data stored in memory as will be used by the vertex shader
    /// </summary>
    /// <returns>A Vulkan-usable struct defining layout of vertex data in memory</returns>
    static VkVertexInputBindingDescription getBindingDescription();
    /// <summary>
    /// Defines how to extract specific vertex attributes from the raw data of a single vertex
    /// </summary>
    /// <returns>A 2 element array defining how position and color should be extracted</returns>
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
};
