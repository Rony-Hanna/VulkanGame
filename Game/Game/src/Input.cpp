#include "Input.h"

std::unordered_map<uint16_t, std::function<void(const InputAction)>> Input::m_Callbacks{};
glm::dvec2 Input::m_MousePos{};

void Input::AddBinding(const uint16_t _key, const std::function<void(const InputAction)>& _callback)
{
	m_Callbacks[_key] = _callback;
}

void Input::RemoveBinding(const uint16_t _key)
{
	m_Callbacks.erase(_key);
}

void Input::NotifyInput(const uint16_t _key, const InputAction _inputAction)
{
	const auto iter = m_Callbacks.find(_key);
	
	if (iter != m_Callbacks.end())
	{
		m_Callbacks[_key](_inputAction);
	}
}

void Input::SetMousePos(const double _x, const double _y)
{
	m_MousePos.x = _x;
	m_MousePos.y = _y;
}

glm::dvec2 Input::GetMousePos()
{
	return m_MousePos;
}
