#pragma once

struct GLFWwindow;

class EventHandler
{
public:
	EventHandler(GLFWwindow* _window);

private:
	static void OnKeyboardEvent(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
	static void OnMouseMoveEvent(GLFWwindow* _window, double _x, double _y);

private:
	GLFWwindow* m_Window;
};
