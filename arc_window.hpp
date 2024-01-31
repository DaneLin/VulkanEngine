#ifndef __ARC_WINDOW_H__
#define __ARC_WINDOW_H__

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace arc
{
    class ArcWindow
    {
    public:
        ArcWindow(int w, int h, std::string name);
        ~ArcWindow();

        ArcWindow(const ArcWindow &) = delete;
        ArcWindow &operator=(const ArcWindow &) = delete;

        bool shouldClose() { return glfwWindowShouldClose(window); }
        VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
        bool wasWindowResized() { return framebufferResized; }
        void resetWindowResizedFlag() { framebufferResized = false; }
        GLFWwindow *getGLFWwindow() const { return window; }

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    private:
        static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
        void initWindow();

        int width;
        int height;
        bool framebufferResized = false;

        std::string windowName;
        GLFWwindow *window;
    };

}
#endif // __ARC_WINDOW_H__