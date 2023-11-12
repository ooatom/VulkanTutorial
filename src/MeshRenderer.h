#include <vector>
#include <array>

#ifndef RENDERER_MESHDRAWER_H
#define RENDERER_MESHDRAWER_H

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include "Renderer.h"

class Application;

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;

    bool operator==(const Vertex &other) const {
        return other.position == position && other.color == color && other.uv == uv;
    }

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription description;
        description.binding = 0;
        description.stride = sizeof(Vertex);
        description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return description;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> descriptions{};
        descriptions[0].binding = 0;
        descriptions[0].location = 0;
        descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        descriptions[0].offset = offsetof(Vertex, position);

        descriptions[1].binding = 0;
        descriptions[1].location = 1;
        descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        descriptions[1].offset = offsetof(Vertex, color);

        descriptions[2].binding = 0;
        descriptions[2].location = 2;
        descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        descriptions[2].offset = offsetof(Vertex, uv);

        return descriptions;
    }
};


namespace std {
    template<>
    struct hash<Vertex> {
        size_t operator()(Vertex const &vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                   (hash<glm::vec2>()(vertex.uv) << 1);
        }
    };
}

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 projection;
};

class MeshRenderer : public Renderer {
public:
    void init(Application *application) override;

    void update(float deltaTime, uint32_t frameNum) override;

    void render(VkCommandBuffer commandBuffer, uint32_t frameNum) override;

    void cleanup() override;

    void createPipeline() override;

    const DescriptorPoolRequirement getDescriptorPoolRequirement() override;

private:
    void createDescriptorSetLayout();

    void createUniformBuffers();

    void createDescriptorSets();

    void createTextureImage();

    void createTextureImageView();

    void createTextureImageSampler();

    void loadModel();

    void createVertexBuffer();

    void createIndexBuffer();

    Application *app;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;

    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBufferMemories;
    std::vector<void *> uniformBufferMemoriesMapped;

//    std::vector<Vertex> vertices = {
//        {{-0.5, -0.5, 0.0},  {1.0, 0.0, 0.0}, {0.0, 1.0}},
//        {{0.5,  -0.5, 0.0},  {0.0, 1.0, 0.0}, {1.0, 1.0}},
//        {{0.5,  0.5,  0.0},  {0.0, 0.0, 1.0}, {1.0, 0.0}},
//        {{-0.5, 0.5,  0.0},  {1.0, 1.0, 1.0}, {0.0, 0.0}},
//
//        {{-0.5, -0.5, -0.5}, {1.0, 0.0, 0.0}, {0.0, 1.0}},
//        {{0.5,  -0.5, -0.5}, {0.0, 1.0, 0.0}, {1.0, 1.0}},
//        {{0.5,  0.5,  -0.5}, {0.0, 0.0, 1.0}, {1.0, 0.0}},
//        {{-0.5, 0.5,  -0.5}, {1.0, 1.0, 1.0}, {0.0, 0.0}},
//    };
//    std::vector<uint16_t> indices = {
//            0, 1, 2, 0, 2, 3,
//            4, 5, 6, 4, 6, 7,
//    };

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    uint32_t mipLevels;
    VkImage textureImage;
    VkImageView textureImageView;
    VkDeviceMemory textureImageMemory;
    VkSampler textureImageSampler;
};

#endif //RENDERER_MESHDRAWER_H
