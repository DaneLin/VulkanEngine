#ifndef __ARC_IMAGE_H__
#define __ARC_IMAGE_H__

#include "arc_device.hpp"

namespace arc
{
    class ArcImage
    {
    public:
        ArcImage(ArcDevice &device);
        ~ArcImage();

        ArcImage(const ArcImage &) = delete;
        ArcImage &operator=(const ArcImage &) = delete;

        void createImage(uint32_t width, uint32_t height, VkFormat format,
                         uint32_t miplevels, uint32_t arrayLayers, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
        void transitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        void createImageView(VkFormat format);

        VkImage getImage() const { return image; }
        VkImageView getImageView() const { return imageView; }

    private:
        ArcDevice &arcDevice;

        VkImage image{};
        VkDeviceMemory imageMemory{};
        VkImageView imageView{};
    };
}

#endif // __ARC_IMAGE_H__