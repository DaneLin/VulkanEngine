#include "first_app.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

namespace arc
{
    struct SimplePushConstantData
    {
        glm::mat2 transform{1.0f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    FirstApp::FirstApp()
    {
        loadGameObjects();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }
    
    FirstApp::~FirstApp()
    {
        vkDestroyPipelineLayout(arcDevice.device(), pipelineLayout, nullptr);
    }

    void FirstApp::run()
    {
        while (!arcWindow.shouldClose())
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(arcDevice.device());
    }
    
    void FirstApp::sierpinski(std::vector<ArcModel::Vertex> &vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top)
    {
        if (depth <= 0)
        {
            vertices.push_back({top});
            vertices.push_back({right});
            vertices.push_back({left});
            return;
        }
        auto leftTop = 0.5f * (left + top);
        auto rightTop = 0.5f * (right + top);
        auto leftRight = 0.5f * (left + right);
        sierpinski(vertices, depth - 1, left, leftRight, leftTop);
        sierpinski(vertices, depth - 1, leftRight, right, rightTop);
        sierpinski(vertices,depth -1, leftTop, rightTop, top);
    }
    
    void FirstApp::loadGameObjects()
    {
        std::vector<ArcModel::Vertex> vertices {
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
        };
        //std::vector<ArcModel::Vertex> vertices{};
        //sierpinski(vertices, 5, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});

        auto arcModel = std::make_shared<ArcModel>(arcDevice, vertices);

        auto triangle = ArcGameObject::createGameObject();
        triangle.model = arcModel;
        triangle.color = {0.1f, 0.8f, 0.1f};
        triangle.transform2D.translation.x = .2f;
        triangle.transform2D.scale = {2.0f, 0.5f};
        triangle.transform2D.rotation = 0.25 * glm::two_pi<float>();

        gameObjects.push_back(std::move(triangle));
    }
    
    void FirstApp::createPipelineLayout()
    {

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if(vkCreatePipelineLayout(arcDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }
    
    void FirstApp::createPipeline()
    {
        assert(arcSwapChain!=nullptr && "Cannot create pipeline before swap chain!");
        assert(pipelineLayout != nullptr && "Cannot craete pipeline before pipeline layout!");
        PipelineConfigInfo pipelineConfig{};
        ArcPipeline::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = arcSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        arcPipeline = std::make_unique<ArcPipeline>(
            arcDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            pipelineConfig
        );
    }
    
    void FirstApp::createCommandBuffers()
    {
        commandBuffers.resize(arcSwapChain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = arcDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(arcDevice.device(), &allocInfo, commandBuffers.data())!= VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }
    
    void FirstApp::freeCommandBuffers()
    {
        vkFreeCommandBuffers(arcDevice.device(), arcDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }
    
    void FirstApp::drawFrame()
    {
        uint32_t imageIndex;
        auto result = arcSwapChain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }  

        recordCommandBuffer(imageIndex);

        result = arcSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || arcWindow.wasWindowResized())
        {
            arcWindow.resetWindowResizedFlag();
            recreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }
    
    void FirstApp::recreateSwapChain()
    {
        auto extent = arcWindow.getExtent();
        while (extent.width ==0 || extent.height == 0)
        {
            extent = arcWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(arcDevice.device());
        
        if (arcSwapChain == nullptr)
        {
            arcSwapChain = std::make_unique<ArcSwapChain>(arcDevice, extent);
        }
        else
        {
            arcSwapChain = std::make_unique<ArcSwapChain>(arcDevice, extent, std::move(arcSwapChain));
            if (arcSwapChain->imageCount() != commandBuffers.size())
            {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        createPipeline();
    }
    
    void FirstApp::recordCommandBuffer(int imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = arcSwapChain->getRenderPass();
            renderPassInfo.framebuffer = arcSwapChain->getFrameBuffer(imageIndex);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = arcSwapChain->getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.height = static_cast<float>(arcSwapChain->getSwapChainExtent().height);
            viewport.width = static_cast<float>(arcSwapChain->getSwapChainExtent().width);
            viewport.minDepth =0.0f;
            viewport.maxDepth = 1.0f;
            VkRect2D scissor{{0, 0}, arcSwapChain->getSwapChainExtent()};
            vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
            vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

            renderGameObjects(commandBuffers[imageIndex]);    

            vkCmdEndRenderPass(commandBuffers[imageIndex]);
            
            if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }
    }
    
    void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer)
    {
        arcPipeline->bind(commandBuffer);

        for (auto& obj : gameObjects)
        {
            obj.transform2D.rotation = glm::mod(obj.transform2D.rotation + 0.01f, glm::two_pi<float>());

            SimplePushConstantData push{};
            push.offset = obj.transform2D.translation;
            push.color = obj.color;
            push.transform = obj.transform2D.mat2();

                vkCmdPushConstants(commandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(SimplePushConstantData),
                    &push);

            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }
}