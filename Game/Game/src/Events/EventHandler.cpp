#include "EventHandler.h"
#include "KeyboardEvent.h"
#include "../Input.h"
#include <glfw3.h>

EventHandler::EventHandler(GLFWwindow* _window) : 
	m_Window(_window)
{
	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	glfwSetKeyCallback(m_Window, OnKeyboardEvent);
	glfwSetCursorPosCallback(m_Window, OnMouseMoveEvent);
}

void EventHandler::OnKeyboardEvent(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
{
	Input::NotifyInput(static_cast<uint16_t>(_key), static_cast<InputAction>(_action));
}

void EventHandler::OnMouseMoveEvent(GLFWwindow* _window, double _x, double _y)
{
	Input::SetMousePos(_x, _y);
}
