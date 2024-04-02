#include "first_app.hpp"
#include "arc_camera.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"
#include "systems/specializationConstants.hpp"
// #include "systems/imgui_system.hpp"
#include "systems/stencil_system.hpp"
#include "keyboard_movement_controller.hpp"
#include "arc_frame_info.hpp"
#include "arc_texture.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <iostream>
#include <chrono>
#include <stdexcept>
#include <array>

namespace arc
{
    FirstApp::FirstApp()
    {
        // global pool for allocating descriptor sets
        globalPool = ArcDescriptorPool::Builder(arcDevice)
                         .setMaxSets(ArcSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ArcSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, ArcSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .build();

        loadGameObjects();
    }

    FirstApp::~FirstApp()
    {
    }

    void FirstApp::run()
    {
        std::vector<std::unique_ptr<ArcBuffer>> globalUboBuffers(ArcSwapChain::MAX_FRAMES_IN_FLIGHT);
        // ArcTexture arcTeture{arcDevice, "images/texture.jpg"};
        ArcTexture arcTeture{arcDevice, "images/viking_room.png"};
        for (int i = 0; i < globalUboBuffers.size(); ++i)
        {
            globalUboBuffers[i] = std::make_unique<ArcBuffer>(
                arcDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            globalUboBuffers[i]->map();
        }

        auto globalSetLayout = ArcDescriptorSetLayout::Builder(arcDevice)
                                   .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                   .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                   .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(ArcSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); ++i)
        {
            auto bufferInfo = globalUboBuffers[i]->descriptorInfo();

            VkDescriptorImageInfo imageInfo = arcTeture.descriptorInfo();

            ArcDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(globalDescriptorSets[i]);
        }

        // SimpleRenderSystem simpleRenderSystem{arcDevice, arcRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        PointLightSystem pointLightSystem{arcDevice, arcRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        // SpecializationConstantSystem specializationConstantSystem{arcDevice, arcRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        StencilSystem stencilSystem{arcDevice, arcRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        ArcCamera camera{};
        // camera.setViewDirection(glm::vec3{0.f}, glm::vec3(0.5f, 0.f, 1.f));

        auto viewObject = ArcGameObject::createGameObject();
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!arcWindow.shouldClose())
        {
            glfwPollEvents();

            // calculate the delta time for game loop
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(arcWindow.getGLFWwindow(), frameTime, viewObject);
            camera.setViewYXZ(viewObject.transform.translation, viewObject.transform.rotation);

            float aspect = arcRenderer.getAspectRatio();
            // camera.setOrthographicProjection(-aspect, aspect, -1, 1, -1, 1);
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.f);
            // each update advanced game time by a certain time
            // it taks a certain amount of real time to process that
            if (auto commandBuffer = arcRenderer.beginFrame())
            {
                int frameIndex = arcRenderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects};

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                globalUboBuffers[frameIndex]->writeToBuffer(&ubo);
                globalUboBuffers[frameIndex]->flush();

                // render
                arcRenderer.beginSwapChainRenderPass(commandBuffer);
                // simpleRenderSystem.renderGameObjects(frameInfo);
                // specializationConstantSystem.renderGameObjects(frameInfo);
                stencilSystem.render(frameInfo);
                pointLightSystem.render(frameInfo);
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
        std::shared_ptr<ArcModel> arcModel;
        // arcModel= ArcModel::createModelFromFile(arcDevice, "models/smooth_vase.obj");
        //  auto gameObj = ArcGameObject::createGameObject();
        //  gameObj.model = arcModel;
        //  gameObj.transform.translation = {0.f, 0.5f, 0.f};
        //  gameObj.transform.scale = {1.0f, 1.5f, 1.0f};
        //  gameObjects.emplace(gameObj.getID(), std::move(gameObj));

        // arcModel = ArcModel::createModelFromFile(arcDevice, "models/flat_vase.obj");
        // auto flatVase = ArcGameObject::createGameObject();
        // flatVase.model = arcModel;
        // flatVase.transform.translation = {.5f, .5f, 1.f};
        // flatVase.transform.scale = {3.0f, 1.6f, 3.f};
        // gameObjects.emplace(flatVase.getID(), std::move(flatVase));

        // arcModel = ArcModel::createModelFromFile(arcDevice, "models/quad.obj");
        // auto floor = ArcGameObject::createGameObject();
        // floor.model = arcModel;
        // floor.transform.translation = {0.f, .5f, 1.f};
        // floor.transform.scale = {3.0f, 1.f, 3.f};
        // gameObjects.emplace(floor.getID(), std::move(floor));

        arcModel = ArcModel::createModelFromFile(arcDevice, "models/venus.obj");
        auto venus = ArcGameObject::createGameObject();
        venus.model = arcModel;
        venus.transform.translation = {1.f, 0.f, 1.f};
        // venus.transform.rotation = {0.f, -1.f, 0.f};
        gameObjects.emplace(venus.getID(), std::move(venus));

        // arcModel = ArcModel::createModelFromFile(arcDevice, "models/viking_room.obj");
        // auto vikingRoom = ArcGameObject::createGameObject();
        // vikingRoom.model = arcModel;
        // vikingRoom.transform.translation = {1.f, 0.f, 0.f};
        // gameObjects.emplace(vikingRoom.getID(), std::move(vikingRoom));

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}};

        for (int i = 0; i < lightColors.size(); i++)
        {
            auto pointLight = ArcGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getID(), std::move(pointLight));
        }
    }
}