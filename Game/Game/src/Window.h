#pragma once

struct GLFWwindow;

class Window
{
public:
	Window(int _width, int _height, const char* _title);
	~Window();

	bool IsOpened();
	bool WasResized() const;
	void SetResized(const bool _isResized);
	void PollEvents();
	GLFWwindow* GetWindow() const;

private:
	static void OnFramebufferResized(GLFWwindow* _window, int _width, int _height);

private:
	int m_Width;
	int m_Height;
	bool m_WindowResized;
	GLFWwindow* m_Window;
};
