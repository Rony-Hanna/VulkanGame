#pragma once

#include <glm/glm.hpp>
#include "Vulkan/VulkanUtilities.h"

enum class InputAction : unsigned short;

struct CameraTransform
{
	glm::mat4 view{ glm::mat4(1.0f) };
	glm::mat4 proj{ glm::mat4(1.0f) };
};

class Camera
{
public:
	Camera();

	void Init();
	void CleanUp();
	void SetView(const glm::vec3& _pos, const glm::vec3& _lookDir, const glm::vec3& _up = glm::vec3(0.0f, 1.0f, 0.0f));
	void SetProjection(const float _fov, const float _aspectRatio, const float _near, const float _far);
	void Update();
	CameraTransform GetCameraTransform() const { return m_CameraTransform; }
	UniformBuffer GetUniformBuffer() const { return m_CameraMatricesBuffer; }

private:
	void CreateUniformBuffer();
	void OnMoveForward(const InputAction _inputAction);
	void OnMoveBackward(const InputAction _inputAction);
	void OnMoveLeft(const InputAction _inputAction);
	void OnMoveRight(const InputAction _inputAction);
	void UpdateMove();
	void UpdateRotation();

private:
	CameraTransform m_CameraTransform;
	UniformBuffer m_CameraMatricesBuffer;
	glm::vec3 m_Position;
	glm::vec3 m_Direction;
	glm::vec3 m_Up;
	glm::dvec2 m_PrevOffset;
	float m_Yaw;
	float m_Pitch;
	bool m_MoveForward;
	bool m_MoveBackward;
	bool m_MoveLeft;
	bool m_MoveRight;
};
