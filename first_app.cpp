#include "first_app.hpp"
#include "simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace arc
{
    FirstApp::FirstApp()
    {
        loadGameObjects();
    }

    FirstApp::~FirstApp()
    {
    }

    void FirstApp::run()
    {
        SimpleRenderSystem simpleRenderSystem{arcDevice, arcRenderer.getSwapChainRenderPass()};
        while (!arcWindow.shouldClose())
        {
            glfwPollEvents();

            if (auto commandBuffer = arcRenderer.beginFrame())
            {
                arcRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
                arcRenderer.endSwapChainRenderPass(commandBuffer);
                arcRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(arcDevice.device());
    }

    void FirstApp::loadGameObjects()
    {
        std::vector<ArcModel::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
        // std::vector<ArcModel::Vertex> vertices{};
        // sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});

        auto arcModel = std::make_shared<ArcModel>(arcDevice, vertices);

        auto triangle = ArcGameObject::createGameObject();
        triangle.model = arcModel;
        triangle.color = {0.1f, 0.8f, 0.1f};
        triangle.transform2D.translation.x = .2f;
        triangle.transform2D.scale = {2.0f, 0.5f};
        triangle.transform2D.rotation = 0.25 * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    }
}