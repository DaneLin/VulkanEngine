#ifndef __ARC_TEXTURE_H__
#define __ARC_TEXTURE_H__

#include "arc_device.hpp"

// std
#include <string>

namespace arc
{
    class ArcTexture
    {
    public:
        ArcTexture(ArcDevice &arcDevice, const std::string &imagepath);
        ~ArcTexture();

        ArcTexture(const ArcTexture &) = delete;
        ArcTexture &operator=(const ArcTexture &) = delete;

        VkDescriptorImageInfo descriptorInfo();

        VkImageView getImageView() const { return textureImageView; }
        VkSampler getSampler() const { return textureSampler; }

    private:
        void createTextureImage(const std::string &imagepath);
        void createImage(uint32_t width, uint32_t height, VkFormat format,
                         VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                         VkImage &image, VkDeviceMemory &imageMemory);

        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        // helper function
        VkImageView createImageView(VkImage image, VkFormat format);
        void createTextureImageView();

        // Sampler
        void createTextureSampler();

    private:
        ArcDevice &arcDevice;

        VkImage textureImage{};
        VkDeviceMemory textureImageMemory{};

        VkImageView textureImageView{};
        // The sampler is a distinct object that provides an interface to extract colors from a texture
        // It can be applied to any image we want
        VkSampler textureSampler{};
        uint32_t mipLevels;
    };
}

#endif // __ARC_TEXTURE_H__