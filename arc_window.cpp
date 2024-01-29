#include "arc_window.hpp"

#include <stdexcept>

namespace arc
{
    ArcWindow::ArcWindow(int w, int h, std::string name) : width(w), height(h), windowName(name)
    {
        initWindow();
    }

    ArcWindow::~ArcWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void ArcWindow::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);

        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }


    void ArcWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }
    
    void ArcWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto arcWindow = reinterpret_cast<ArcWindow*>(glfwGetWindowUserPointer(window));
        arcWindow->framebufferResized = true;
        arcWindow->width = width;
        arcWindow->height = height;
    }
    
}