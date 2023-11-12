#ifndef RENDERER_PARTICLERENDERER_H
#define RENDERER_PARTICLERENDERER_H

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "Renderer.h"

class Application;

struct DescriptorPoolRequirement;

struct ParticleUniformBufferObject {
    glm::float32 deltaTime;
};

struct Particle {
    alignas(8) glm::vec2 position;
    alignas(8) glm::vec2 velocity;
    alignas(16) glm::vec4 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = static_cast<uint32_t>(sizeof(Particle));
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].offset = offsetof(Particle, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].offset = offsetof(Particle, color);

        return attributeDescriptions;
    }
};

class ParticleRenderer : public Renderer {
public:
    int PARTICLE_COUNT = 1000;

    ParticleRenderer() {
        needCompute = true;
        graphicsWaitComputeStage = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
    }

    void init(Application *application) override;

    void update(float deltaTime, uint32_t frameNum) override;

    void compute(VkCommandBuffer commandBuffer, uint32_t frameNum) override;

    void render(VkCommandBuffer commandBuffer, uint32_t frameNum) override;

    void cleanup() override;

    void createPipeline() override;

    const DescriptorPoolRequirement getDescriptorPoolRequirement() override;

private:
    Application *app;
    VkDescriptorSetLayout computeDescriptorSetLayout;
    std::vector<VkDescriptorSet> computeDescriptorSets;

    VkPipelineLayout computePipelineLayout;
    VkPipeline computePipeline;
    VkPipelineLayout graphicsPipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<Particle> particles;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBufferMemories;
    std::vector<void *> uniformBufferMemoriesMapped;
    std::vector<VkBuffer> shaderStorageBuffers;
    std::vector<VkDeviceMemory> shaderStorageBufferMemories;

    void createParticleData();

    void createDescriptorSetLayout();

    void createUniformBuffers();

    void createShaderStorageBuffers();

    void createDescriptorSets();
};


#endif //RENDERER_PARTICLERENDERER_H
