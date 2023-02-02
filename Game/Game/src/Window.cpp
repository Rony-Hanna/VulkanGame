#include "Window.h"
#include <glfw3.h>
#include <stdexcept>

Window::Window(int _width, int _height, const char* _title) : 
	m_Width(_width),
	m_Height(_height),
	m_WindowResized(false),
	m_Window(nullptr)
{
	if (!glfwInit())
	{
		throw std::runtime_error("GLFW ERROR: Failed to initialize library\n");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	m_Window = glfwCreateWindow(_width, _height, _title, nullptr, nullptr);

	if (!m_Window)
	{
		throw std::runtime_error("GLFW ERROR: Failed to create window\n");
	}

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, OnFramebufferResized);
}

Window::~Window()
{
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

bool Window::IsOpened()
{
	return !glfwWindowShouldClose(m_Window);
}

bool Window::WasResized() const
{
	return m_WindowResized;
}

void Window::SetResized(const bool _isResized)
{
	m_WindowResized = _isResized;
}

void Window::PollEvents()
{
	glfwPollEvents();
}

GLFWwindow* Window::GetWindow() const
{
	return m_Window;
}

void Window::OnFramebufferResized(GLFWwindow* _window, int _width, int _height)
{
	auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(_window));
	
	if (window)
	{
		window->SetResized(true);
	}
}
