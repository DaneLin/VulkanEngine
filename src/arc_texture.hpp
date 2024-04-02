#ifndef __ARC_TEXTURE_H__
#define __ARC_TEXTURE_H__

#include "arc_device.hpp"
#include "arc_image.hpp"
// std
#include <string>
#include <memory>

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

        VkSampler getSampler() const { return textureSampler; }

    private:
        void createTextureImage(const std::string &imagepath);

        // helper function
        void createImageView(VkFormat format);

        // Sampler
        void createTextureSampler();

    private:
        ArcDevice &arcDevice;

        std::unique_ptr<ArcImage> arcImage;
        // The sampler is a distinct object that provides an interface to extract colors from a texture
        // It can be applied to any image we want
        VkSampler textureSampler{};
        uint32_t mipLevels;
    };
}

#endif // __ARC_TEXTURE_H__