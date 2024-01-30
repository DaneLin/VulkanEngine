#include "arc_renderer.hpp"
#include <stdexcept>
#include <array>

namespace arc
{
    ArcRenderer::ArcRenderer(ArcWindow &window, ArcDevice &device) : arcWindow(window), arcDevice(device)
    {
        recreateSwapChain();
        createCommandBuffers();
    }

    VkCommandBuffer ArcRenderer::beginFrame()
    {
        assert(!isFrameStarted && "Cannot call beginFrame while already in progress!");

        // one to one, point to point
        auto result = arcSwapChain->acquireNextImage(&currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        return commandBuffer;
    }

    void ArcRenderer::endFrame()
    {
        assert(isFrameStarted && "Cannot call endFrame while frame is not in progress!");
        auto commandBuffer = getCurrentCommandBuffer();

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }

        auto result = arcSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || arcWindow.wasWindowResized())
        {
            arcWindow.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % ArcSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void ArcRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Cannot call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = arcSwapChain->getRenderPass();
        renderPassInfo.framebuffer = arcSwapChain->getFrameBuffer(currentImageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = arcSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.height = static_cast<float>(arcSwapChain->getSwapChainExtent().height);
        viewport.width = static_cast<float>(arcSwapChain->getSwapChainExtent().width);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, arcSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void ArcRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Cannot call endSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Cannot end render pass on command buffer from a different frame");

        vkCmdEndRenderPass(commandBuffer);
    }

    ArcRenderer::~ArcRenderer()
    {
        freeCommandBuffers();
    }

    void ArcRenderer::createCommandBuffers()
    {
        commandBuffers.resize(ArcSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = arcDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(arcDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void ArcRenderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(arcDevice.device(), arcDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    void ArcRenderer::recreateSwapChain()
    {
        auto extent = arcWindow.getExtent();
        while (extent.width == 0 || extent.height == 0)
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
            std::shared_ptr<ArcSwapChain> oldSwapChain = std::move(arcSwapChain);
            arcSwapChain = std::make_unique<ArcSwapChain>(arcDevice, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormat(*arcSwapChain.get()))
            {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }
        }
    }
}