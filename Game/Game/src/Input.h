#pragma once

#include "Events/KeyboardEvent.h"
#include <functional>
#include <glm/glm.hpp>

class Input
{
public:
	static void AddBinding(const uint16_t _key, const std::function<void(const InputAction)>& _callback);
	static void RemoveBinding(const uint16_t _key);
	static void NotifyInput(const uint16_t _key, const InputAction _inputAction);
	static void SetMousePos(const double _x, const double _y);
	static glm::dvec2 GetMousePos();

private:
	static std::unordered_map<uint16_t, std::function<void(const InputAction)>> m_Callbacks;
	static glm::dvec2 m_MousePos;
};
