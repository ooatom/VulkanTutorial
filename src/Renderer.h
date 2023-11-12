#ifndef RENDERER_RENDERER_H
#define RENDERER_RENDERER_H

#include <vulkan/vulkan.h>
#include <vector>
#include <array>

class Application;

struct DescriptorPoolRequirement;

class Renderer {
public:
    bool needCompute = false;
    VkPipelineStageFlagBits graphicsWaitComputeStage = VK_PIPELINE_STAGE_NONE;

    virtual void init(Application *application) = 0;

    virtual void update(float deltaTime, uint32_t frameNum) = 0;

    virtual void compute(VkCommandBuffer commandBuffer, uint32_t frameNum) {};

    virtual void render(VkCommandBuffer commandBuffer, uint32_t frameNum) = 0;

    virtual void cleanup() = 0;

    virtual void createPipeline() = 0;

    virtual const DescriptorPoolRequirement getDescriptorPoolRequirement() = 0;

    virtual ~Renderer() = default;
};

#endif //RENDERER_RENDERER_H
