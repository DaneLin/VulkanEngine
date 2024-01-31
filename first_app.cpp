#include "first_app.hpp"
#include "arc_camera.hpp"
#include "simple_render_system.hpp"
#include "keyboard_movement_controller.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <chrono>
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
        ArcCamera camera{};
        // camera.setViewDirection(glm::vec3{0.f}, glm::vec3(0.5f, 0.f, 1.f));
        camera.setViewTarget(glm::vec3(-1.f, -2.f, -2.f), glm::vec3(0.f, 0.f, 2.5f));

        auto viewObject = ArcGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!arcWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();

            currentTime = newTime;

            cameraController.moveInPlaneXZ(arcWindow.getGLFWwindow(), frameTime, viewObject);
            camera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);

            float aspect = arcRenderer.getAspectRatio();
            // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.f);

            if (auto commandBuffer = arcRenderer.beginFrame())
            {
                arcRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                arcRenderer.endSwapChainRenderPass(commandBuffer);
                arcRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(arcDevice.device());
    }

    std::unique_ptr<ArcModel> createCubeModel(ArcDevice &device, glm::vec3 offset)
    {
        ArcModel::Builder modelBuilder{};
        modelBuilder.vertices = {
            // left face
            {{-0.5f, -0.5f, -0.5f}, {0.9f, 0.9f, 0.9f}},
            {{-0.5f, 0.5f, 0.5f}, {0.9f, 0.9f, 0.9f}},
            {{-0.5f, -0.5f, 0.5f}, {0.9f, 0.9f, 0.9f}},
            {{-0.5f, 0.5f, -0.5f}, {0.9f, 0.9f, 0.9f}},

            // right face
            {{0.5f, -0.5f, -0.5f}, {0.8f, 0.9f, 0.1f}},
            {{0.5f, 0.5f, 0.5f}, {0.8f, 0.9f, 0.1f}},
            {{0.5f, -0.5f, 0.5f}, {0.8f, 0.9f, 0.1f}},
            {{0.5f, 0.5f, -0.5f}, {0.8f, 0.9f, 0.1f}},

            // top face (orange, remember y axis points down)
            {{-0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.1f}},
            {{0.5f, -0.5f, 0.5f}, {0.8f, 0.8f, 0.1f}},
            {{-0.5f, -0.5f, 0.5f}, {0.8f, 0.8f, 0.1f}},
            {{0.5f, -0.5f, -0.5f}, {0.9f, 0.6f, 0.1f}},

            // bottom face
            {{-0.5f, 0.5f, -0.5f}, {0.9f, 0.6f, 0.1f}},
            {{0.5f, 0.5f, 0.5f}, {0.8f, 0.8f, 0.1f}},
            {{-0.5f, 0.5f, 0.5f}, {0.8f, 0.8f, 0.1f}},
            {{0.5f, 0.5f, -0.5f}, {0.9f, 0.6f, 0.1f}},

            // nose face
            {{-0.5f, -0.5f, 0.5f}, {0.1f, 0.1f, 0.8f}},
            {{0.5f, 0.5f, 0.5f}, {0.1f, 0.1f, 0.8f}},
            {{-0.5f, 0.5f, 0.5f}, {0.1f, 0.1f, 0.8f}},
            {{0.5f, -0.5f, 0.5f}, {0.1f, 0.1f, 0.8f}},

            // tail face
            {{-0.5f, -0.5f, -0.5f}, {0.1f, 0.8f, 0.1f}},
            {{0.5f, 0.5f, -0.5f}, {0.1f, 0.8f, 0.1f}},
            {{-0.5f, 0.5f, -0.5f}, {0.1f, 0.8f, 0.1f}},
            {{0.5f, -0.5f, -0.5f}, {0.1f, 0.8f, 0.1f}},
        };

        for (auto &v : modelBuilder.vertices)
        {
            v.position += offset;
        }

        modelBuilder.indices = {0, 1, 2, 0, 3, 1, 4, 5, 6, 4, 7, 5, 8, 9, 10, 8, 11, 9, 12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};

        return std::make_unique<ArcModel>(device, modelBuilder);
    }

    void FirstApp::loadGameObjects()
    {
        std::shared_ptr<ArcModel> arcModel = ArcModel::createModelFromFile(arcDevice, "models/flat_vase.obj");

        auto gameObj = ArcGameObject::createGameObject();
        gameObj.model = arcModel;
        gameObj.transform.translation = {0.f, 0.5f, 2.5f};
        gameObj.transform.scale = {1.0f, 1.5f, 1.0f};
        gameObjects.push_back(std::move(gameObj));
    }
}