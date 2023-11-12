#include <vector>
#include <iostream>
#include <optional>

#ifndef RENDERER_APPLICATION_H
#define RENDERER_APPLICATION_H

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <GLFW/glfw3.h>

#include "Renderer.h"
#include "MeshRenderer.h"
#include "ParticleRenderer.h"

static void checkVkResult(VkResult result) {
    if (result == VK_SUCCESS) return;
    std::cout << "[vulkan] Error: VkResult = " << string_VkResult(result) << "\n";
    if (result < 0)
        abort();
}

struct DescriptorPoolRequirement {
    std::vector<VkDescriptorPoolSize> poolSizes;
    uint32_t maxSets;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsComputeFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsComputeFamily.has_value() && presentFamily.has_value();
//        return graphicsComputeFamily.has_value() && graphicsComputeFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};


class Application {
public:
    void run();

    static const int MAX_FRAMES_IN_FLIGHT = 2;

    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkSwapchainKHR swapChain;

    VkQueue graphicsComputeQueue;
    VkQueue presentQueue;

    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkRenderPass renderPass;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkDescriptorPool descriptorPool;

    VkCommandPool transientCommandPool;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> computeCommandBuffers;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkSemaphore> computeFinishedSemaphores;
    std::vector<VkFence> computeInFlightFences;
    uint32_t currentFrame;

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags,
                      VkMemoryPropertyFlags memoryPropertyFlags, VkBuffer &buffer,
                      VkDeviceMemory &deviceMemory);

    void generateMipmaps(VkImage image, VkFormat format, int32_t width, int32_t height, uint32_t mips);

    void createImage(int width, int height, uint32_t mips, VkSampleCountFlagBits numSamples, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryPropertyFlags,
                     VkImage &image, VkDeviceMemory &imageMemory);

    void createImageView(VkImage image, VkFormat format, VkImageView &imageView, VkImageAspectFlags aspectFlags,
                         uint32_t mips);

    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t mips);

    void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, int width, int height);

    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    VkShaderModule createShaderModule(const std::vector<char> &code);

private:
    inline static const char *Renderers[] = {"Mesh", "Particle"};
    int rendererIndex = 0;
    MeshRenderer *meshDrawer = new MeshRenderer();
    ParticleRenderer *particleDrawer = new ParticleRenderer();

    Renderer *getRenderer() {
        if (Renderers[rendererIndex] == std::string("Mesh")) {
            return meshDrawer;
        }
        return particleDrawer;
    }

    bool framebufferResized = false;

    void initWindow();

    void initVulkan();

    void initImGui();

    void mainLoop();

    void updateData();

    void cleanup();

    void createInstance();

    void setupDebugMessenger();

    void createSurface();

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createSwapChainImageViews();

    void createCommandPool();

    void createCommandBuffer();

    void createRenderPass();

    void createColorResources();

    void createDepthResources();

    void createFramebuffers();

    void createDescriptorPool(const std::vector<DescriptorPoolRequirement> &poolRequirements);

    void createSyncObjects();

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

    const std::vector<const char *> getRequiredExtensions();

    uint32_t findMemoryTypeIndex(uint32_t typeBitsFilter, VkMemoryPropertyFlags propertyFlags);

    void cleanupSwapChain();

    void recreateSwapChain();

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);

    void createSwapChain(VkSwapchainKHR oldSwapChain = VK_NULL_HANDLE);

    void drawFrame();

    void recordCommandBuffer(VkCommandBuffer currCommandBuffer, uint32_t imageIndex);

    void recordComputeCommandBuffer(VkCommandBuffer currCommandBuffer);

    VkSampleCountFlagBits getMaxUsableSampleCount();

    VkFormat findDepthFormat();

    VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlagBits features);

    bool hasStencilComponent(VkFormat format);

    bool checkValidationLayerSupport();

    int ratePhysicalDeviceSuitability(const VkPhysicalDevice &targetPhysicalDevice);

    QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice &targetPhysicalDevice);

    bool checkDeviceExtensionSupport(const VkPhysicalDevice &targetPhysicalDevice);

    SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice &targetPhysicalDevice);

    VkSurfaceFormatKHR
    chooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

    VkPresentModeKHR
    chooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                        unsigned int messageType,
//                                                        VkDebugUtilsMessageTypeFlagBitsEXT messageType,
                                                        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                        void *pUserData) {
        std::cerr << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }
};

#endif //RENDERER_APPLICATION_H
