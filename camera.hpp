#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Camera {
public:
	/// <summary>
	/// Set projection matrix to an orthographic projection matrix mapping from the axis-aligned bounding box
	/// represented by the given points to Vulkan's canonical view volume
	/// </summary>
	/// <param name="left">X coordinate of minimum x-axis bounding plane</param>
	/// <param name="right">X coordinate of maximum x-axis bounding plane</param>
	/// <param name="top">Y coordinate of minimum y-axis bounding plane (positive Y is towards the bottom in Vulkan)</param>
	/// <param name="bottom">Y coordinate of the maximum y-axis bounding plane (positive Y is towards the bottom in Vulkan)</param>
	/// <param name="near">Z coordinate of the minimum z-axis bounding plane</param>
	/// <param name="far">Z coorindate of the maximum z-axis bounding plane</param>
	void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
	/// <summary>
	/// Set project matrix to a combined perspective-orthographics matrix mapping from the viewing frustum represented by
	/// the given inputs to Vulkan's canonical view volume, assuming that frustum is centered on the z-axis
	/// (which is a valid assumption given Vulkan's canonical view colume)
	/// </summary>
	/// <param name="fov_y">Vertical field of view of the frustum in radians</param>
	/// <param name="aspect_ratio">Aspect ration of the frustum as width/height</param>
	/// <param name="near">Distance of near plane</param>
	/// <param name="">Distance of far plane</param>
	void setPerspectiveProjection(float fov_y, float aspect_ratio, float near, float far);

	const glm::mat4& getProjectionMatrix() const { return projection_matrix; }

private:
	glm::mat4 projection_matrix{ 1.0f };
};
