#pragma once

#include "arc_window.hpp"
#include "arc_device.hpp"
#include "arc_swap_chain.hpp"

// std
#include <vector>
#include <cassert>

namespace arc
{
    class ArcRenderer
    {
    public:
        ArcRenderer(ArcWindow &window, ArcDevice &device);
        ~ArcRenderer();

        ArcRenderer(const ArcRenderer &) = delete;
        ArcRenderer operator=(const ArcRenderer &) = delete;

        VkRenderPass getSwapChainRenderPass() const { return arcSwapChain->getRenderPass(); }

        bool isFrameInProgress() const { return isFrameStarted; }

        VkCommandBuffer getCurrentCommandBuffer() const
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress!");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const
        {
            assert(isFrameStarted && "Cannot get frame index when frame is not in progress!");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();

        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

    private:
        ArcWindow &arcWindow;
        ArcDevice &arcDevice;
        std::unique_ptr<ArcSwapChain> arcSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex = 0;
        bool isFrameStarted = false;
    };
} // namespace arc
