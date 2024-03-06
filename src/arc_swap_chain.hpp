#ifndef __ARC_SWAP_CHAIN_H__
#define __ARC_SWAP_CHAIN_H__

#include "arc_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace arc
{

    class ArcSwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        ArcSwapChain(ArcDevice &deviceRef, VkExtent2D windowExtent);
        ArcSwapChain(ArcDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<ArcSwapChain> previous);
        ~ArcSwapChain();

        ArcSwapChain(const ArcSwapChain &) = delete;
        ArcSwapChain operator=(const ArcSwapChain &) = delete;

        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        VkRenderPass getRenderPass() { return renderPass; }
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }
        size_t imageCount() { return swapChainImages.size(); }
        VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }
        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }

        float extentAspectRatio()
        {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }
        VkFormat findDepthFormat();

        VkResult acquireNextImage(uint32_t *imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

        bool compareSwapFormat(const ArcSwapChain &swapChain) const
        {
            return swapChain.swapChainImageFormat == swapChainImageFormat &&
                   swapChain.swapChainDepthFormat == swapChainDepthFormat;
        }

        // MSAA
        void createColorResources();

    private:
        void init();
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        VkFormat swapChainImageFormat;
        VkFormat swapChainDepthFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;

        // MSAA
        // Each pixel is sampled in an offscreen buffer which is then rendered to the screen
        // This new buffer have to be able to store more than one sample per pixel
        // Once a multisampled buffer is created, it has to be resolved to the default framebuffer
        // which stores only a single sample per pixel
        VkSampleCountFlagBits msaaSamples;
        std::vector<VkImage> colorImages;
        std::vector<VkDeviceMemory> colorImageMemorys;
        std::vector<VkImageView> colorImageViews;

        std::vector<VkImage> depthImages;
        std::vector<VkDeviceMemory> depthImageMemorys;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        ArcDevice &device;
        VkExtent2D windowExtent;

        VkSwapchainKHR swapChain;
        std::shared_ptr<ArcSwapChain> oldSwapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t currentFrame = 0;
    };

} // namespace Arc

#endif // __ARC_SWAP_CHAIN_H__