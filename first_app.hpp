#pragma once


#include "arc_window.hpp"
#include "arc_pipeline.hpp"
#include "arc_device.hpp"
#include "arc_swap_chain.hpp"
#include "arc_game_object.hpp"

// std
#include <vector>

namespace arc
{
    class FirstApp
    {
        public:
            static constexpr int WIDTH = 800;
            static constexpr int HEIGHT = 600;

            FirstApp();
            ~FirstApp();

            FirstApp(const FirstApp &) = delete;
            FirstApp operator=(const FirstApp &) = delete;

            void run();
        
        private:
            void sierpinski(std::vector<ArcModel::Vertex> &vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top);
            void loadGameObjects();
            void createPipelineLayout();
            void createPipeline();
            void createCommandBuffers();
            void freeCommandBuffers();
            void drawFrame();       
            void recreateSwapChain();
            void recordCommandBuffer(int imageIndex);  
            void renderGameObjects(VkCommandBuffer commandBuffer);   
        private:
            ArcWindow arcWindow{WIDTH, HEIGHT, "Hello Vulkan!"};
            ArcDevice arcDevice{arcWindow};
            std::unique_ptr<ArcSwapChain> arcSwapChain;
            std::unique_ptr<ArcPipeline> arcPipeline;
            VkPipelineLayout pipelineLayout;
            std::vector<VkCommandBuffer> commandBuffers;
            std::vector<ArcGameObject> gameObjects;
    };  
} // namespace arc
