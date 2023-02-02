#include "Camera.h"
#include "Vulkan/VulkanUtilities.h"
#include "Input.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() :
	m_CameraTransform{},
	m_CameraMatricesBuffer{},
	m_Position(0.0f),
	m_Direction(0.0f),
	m_Up(0.0f),
	m_PrevOffset(0.0f),
	m_Yaw(-90.0f),
	m_Pitch(0.0f),
	m_MoveForward(false),
	m_MoveBackward(false),
	m_MoveLeft(false),
	m_MoveRight(false)
{}

void Camera::Init()
{
	CreateUniformBuffer();

	Input::AddBinding(INPUT_KEY_W, std::bind(&Camera::OnMoveForward, this, std::placeholders::_1));
	Input::AddBinding(INPUT_KEY_S, std::bind(&Camera::OnMoveBackward, this, std::placeholders::_1));
	Input::AddBinding(INPUT_KEY_A, std::bind(&Camera::OnMoveLeft, this, std::placeholders::_1));
	Input::AddBinding(INPUT_KEY_D, std::bind(&Camera::OnMoveRight, this, std::placeholders::_1));
}

void Camera::CleanUp()
{
	Input::RemoveBinding(INPUT_KEY_W);
	Input::RemoveBinding(INPUT_KEY_S);
	Input::RemoveBinding(INPUT_KEY_A);
	Input::RemoveBinding(INPUT_KEY_D);

	VulkanUtilities::DestroyBuffer(m_CameraMatricesBuffer.buffer, m_CameraMatricesBuffer.bufferMemory);
}

void Camera::CreateUniformBuffer()
{
	BufferInfo bufferInfo{};
	bufferInfo.bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferInfo.bufferSize = sizeof(CameraTransform);
	bufferInfo.pBuffer = &m_CameraMatricesBuffer.buffer;
	bufferInfo.pBufferMemory = &m_CameraMatricesBuffer.bufferMemory;
	bufferInfo.memoryProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	VulkanUtilities::CreateBuffer(bufferInfo);
}

void Camera::OnMoveForward(const InputAction _inputAction)
{
	if (_inputAction == InputAction::Up)
	{
		m_MoveForward = false;
		return;
	}

	m_MoveForward = true;
}

void Camera::OnMoveBackward(const InputAction _inputAction)
{
	if (_inputAction == InputAction::Up)
	{
		m_MoveBackward = false;
		return;
	}

	m_MoveBackward = true;
}

void Camera::OnMoveLeft(const InputAction _inputAction)
{
	if (_inputAction == InputAction::Up)
	{
		m_MoveLeft = false;
		return;
	}

	m_MoveLeft = true;
}

void Camera::OnMoveRight(const InputAction _inputAction)
{
	if (_inputAction == InputAction::Up)
	{
		m_MoveRight = false;
		return;
	}

	m_MoveRight = true;
}

void Camera::SetView(const glm::vec3& _pos, const glm::vec3& _lookDir, const glm::vec3& _up)
{
	m_Position = _pos;
	m_Direction = _lookDir;
	m_Up = _up;

	m_CameraTransform.view = glm::lookAt(m_Position, m_Position + m_Direction, m_Up);
}

void Camera::SetProjection(const float _fov, const float _aspectRatio, const float _near, const float _far)
{
	m_CameraTransform.proj = glm::perspective(_fov, _aspectRatio, _near, _far);
}

void Camera::UpdateMove()
{
	m_CameraTransform.view = glm::lookAt(m_Position, m_Position + m_Direction, m_Up);
}

void Camera::UpdateRotation()
{
	const glm::dvec2 mousePos = Input::GetMousePos();

	double offsetX = mousePos.x - m_PrevOffset.x;
	double offsetY = mousePos.y - m_PrevOffset.y;
	m_PrevOffset.x = mousePos.x;
	m_PrevOffset.y = mousePos.y;

	const double sensitivity = 0.1;
	offsetX *= sensitivity;
	offsetY *= sensitivity;

	m_Yaw += static_cast<float>(offsetX);
	m_Pitch += static_cast<float>(offsetY);

	if (m_Pitch > 89.0f) m_Pitch = 89.0f;
	else if (m_Pitch < -89.0f) m_Pitch = -89.0f;

	glm::vec3 direction = glm::vec3(1.0f);
	direction.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	direction.y = sin(glm::radians(m_Pitch));
	direction.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	m_Direction = glm::normalize(direction);
}

void Camera::Update()
{
	UpdateRotation();

	if (m_MoveForward)	m_Position += m_Direction * 0.001f;
	if (m_MoveBackward) m_Position -= m_Direction * 0.001f;
	if (m_MoveRight)	m_Position += glm::normalize(glm::cross(m_Direction, m_Up)) * 0.001f;
	if (m_MoveLeft)		m_Position -= glm::normalize(glm::cross(m_Direction, m_Up)) * 0.001f;

	UpdateMove();

	void* pData = nullptr;
	VulkanUtilities::MapMemory(m_CameraMatricesBuffer.bufferMemory, sizeof(CameraTransform), &pData);
	memcpy(pData, &m_CameraTransform, sizeof(CameraTransform));
	VulkanUtilities::UnmapMemory(m_CameraMatricesBuffer.bufferMemory);
}
