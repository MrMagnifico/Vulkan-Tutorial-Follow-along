#pragma once

#include "model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>

/// <summary>
/// Defines transformation to be applied to an object to render it in the desired size and position.
/// Final matrix is in left-to-right order (translate -> rotate_x -> rotate_y -> rotate_z -> scale)
/// </summary>
struct TransformComponent {
	glm::vec3 translation{};
	glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
	glm::vec3 rotation{};

    /// <summary>
    /// Combine all transformations represented by components into a single 3D affine transformation matrix.
    /// Rotations correspond to Tait-bryan angles of X(1), Y(2), Z(3)
    /// See (https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix)
    /// </summary>
    /// <returns>4x4 matrix representing affine transformation for application on homogenous coordinates</returns>
    glm::mat4 affineMatrix() {
        const float c1 = glm::cos(rotation.x);
        const float s1 = glm::sin(rotation.x);
        const float c2 = glm::cos(rotation.y);
        const float s2 = glm::sin(rotation.y);
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        return glm::mat4{
            {
                scale.x * (c2 * c2),
                scale.x * (c1 * s3 + c3 * s1 * s2),
                scale.x * (s1 * s3 - c1 * c3 * s2),
                0.0f,
            },
            {
                scale.y * (-c2 * s3),
                scale.y * (c1 * c3 - s1 * s2 * s3),
                scale.y * (c3 * s1 + c1 * s2 * s3),
                0.0f,
            },
            {
                scale.z * (s2),
                scale.z * (-c2 * s1),
                scale.z * (c1 * c2),
                0.0f,
            },
            {translation.x, translation.y, translation.z, 1.0f} };
    }
};

/// <summary>
/// Represents a single object in the scene
/// </summary>
class SceneObject {
public:
	using id_t = uint32_t;

    std::shared_ptr<Model> model{};
    TransformComponent transformation{};

	/// <summary>
	/// Create object with no associated model
	/// </summary>
	SceneObject() { id = current_id++; }
    /// <summary>
    /// Create a scene object with an associated model
    /// </summary>
    /// <param name="model">Model of object</param>
    SceneObject(std::shared_ptr<Model> model) : model{ model } { id = current_id++; }
    /// <summary>
    /// Create a scene object with an associated model and transformation
    /// </summary>
    /// <param name="model">Model of object</param>
    /// <param name="transformation">Transformation to apply on object in the scene</param>
    SceneObject(std::shared_ptr<Model> model, TransformComponent transformation) : model{ model }, transformation{ transformation } { id = current_id++; }

	id_t getId() { return id; }

private:
    inline static id_t current_id = 0;

	id_t id;
};
