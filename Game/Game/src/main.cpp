#include "Window.h"
#include "Events/EventHandler.h"
#include "Vulkan/VulkanRenderer.h"
#include <iostream>

int main()
{
	try
	{
		Window window(800, 600, "Game");
		VulkanRenderer renderer(&window);
		EventHandler eventHandler(window.GetWindow());

		while (window.IsOpened())
		{
			window.PollEvents();
			renderer.Draw();
		}
	}
	catch (std::exception& _ex)
	{
		std::cout << _ex.what() << '\n';
	}
}