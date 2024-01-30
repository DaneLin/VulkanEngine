#pragma once

#include "arc_window.hpp"
#include "arc_device.hpp"
#include "arc_game_object.hpp"
#include "arc_renderer.hpp"

// std
#include <vector>

namespace arc
{
    class FirstApp
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp &) = delete;
        FirstApp operator=(const FirstApp &) = delete;

        void run();

    private:
        void sierpinski(std::vector<ArcModel::Vertex> &vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top);
        void loadGameObjects();

    private:
        ArcWindow arcWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
        ArcDevice arcDevice{arcWindow};
        ArcRenderer arcRenderer{arcWindow, arcDevice};
        std::vector<ArcGameObject> gameObjects;
    };
} // namespace arc
