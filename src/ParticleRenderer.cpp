#include "ParticleRenderer.h"

#include <random>

#include "Application.h"
#include "utils.h"

void ParticleRenderer::init(Application *application) {
    app = application;

    createDescriptorSetLayout();
    createPipeline();

    createParticleData();
    createUniformBuffers();
    createShaderStorageBuffers();
    createDescriptorSets();
}

const DescriptorPoolRequirement ParticleRenderer::getDescriptorPoolRequirement() {
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes{2};

    descriptorPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSizes[0].descriptorCount = static_cast<uint32_t>(app->MAX_FRAMES_IN_FLIGHT);
    descriptorPoolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorPoolSizes[1].descriptorCount = static_cast<uint32_t>(app->MAX_FRAMES_IN_FLIGHT) * 2;

    return {descriptorPoolSizes, static_cast<uint32_t>(app->MAX_FRAMES_IN_FLIGHT)};
}

void ParticleRenderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding timeBinding{};
    timeBinding.binding = 0;
    timeBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    timeBinding.descriptorCount = 1;
    timeBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding ssboBindingIn{};
    ssboBindingIn.binding = 1;
    ssboBindingIn.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssboBindingIn.descriptorCount = 1;
    ssboBindingIn.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutBinding ssboBindingOut{};
    ssboBindingOut.binding = 2;
    ssboBindingOut.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ssboBindingOut.descriptorCount = 1;
    ssboBindingOut.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> setLayoutBindings{timeBinding, ssboBindingIn, ssboBindingOut};

    VkDescriptorSetLayoutCreateInfo computeSetLayoutCreateInfo{};
    computeSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    computeSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
    computeSetLayoutCreateInfo.pBindings = setLayoutBindings.data();

    if (vkCreateDescriptorSetLayout(app->device, &computeSetLayoutCreateInfo, nullptr, &computeDescriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void ParticleRenderer::createPipeline() {
    auto computeShaderCode = readFile("shaders/particle.comp.spv");

    VkShaderModule computeShaderModule = app->createShaderModule(computeShaderCode);

    VkPipelineShaderStageCreateInfo computePipelineShaderStageCreateInfo{};
    computePipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computePipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computePipelineShaderStageCreateInfo.module = computeShaderModule;
    computePipelineShaderStageCreateInfo.pName = "main";

    VkPipelineLayoutCreateInfo computePipelineLayoutCreateInfo{};
    computePipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computePipelineLayoutCreateInfo.pSetLayouts = &computeDescriptorSetLayout;
    computePipelineLayoutCreateInfo.setLayoutCount = 1;
    computePipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    computePipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(app->device, &computePipelineLayoutCreateInfo,
                               nullptr, &computePipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline layout!");
    }

    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.stage = computePipelineShaderStageCreateInfo;
    computePipelineCreateInfo.layout = computePipelineLayout;
//    computePipelineCreateInfo.flags
    computePipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    computePipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateComputePipelines(app->device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo,
                                 nullptr, &computePipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }

    vkDestroyShaderModule(app->device, computeShaderModule, nullptr);


    auto vertShaderCode = readFile("shaders/particle.vert.spv");
    auto fragShaderCode = readFile("shaders/particle.frag.spv");

    VkShaderModule vertShaderModule = app->createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = app->createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    vertShaderStageCreateInfo.flags
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName = "main";
    vertShaderStageCreateInfo.pSpecializationInfo = VK_NULL_HANDLE;

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName = "main";
    fragShaderStageCreateInfo.pSpecializationInfo = VK_NULL_HANDLE;

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStageCreateInfos{vertShaderStageCreateInfo,
                                                                          fragShaderStageCreateInfo};

    auto inputBindingDescription = Particle::getBindingDescription();
    auto inputAttributeDescriptions = Particle::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//    vertexInputStateCreateInfo.flags
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &inputBindingDescription;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(inputAttributeDescriptions.size());
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = inputAttributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    inputAssemblyStateCreateInfo.flags
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo{};
    tessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
//    tessellationStateCreateInfo.flags
//    tessellationStateCreateInfo.patchControlPoints

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    rasterizationStateCreateInfo.flags
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.lineWidth = 1.0;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasClamp = 0.0;
    rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0;

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    colorBlendAttachmentState.blendEnable = VK_TRUE;
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                               VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    colorBlendStateCreateInfo.flags
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    // corresponding to renderPass subPass pColorAttachments
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
    colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
    colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};
    depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    depthStencilStateCreateInfo.flags
    depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.front = {};
    depthStencilStateCreateInfo.back = {};
    // only keep fragments that fall within the specified depth range
    depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.minDepthBounds = 0.0;
    depthStencilStateCreateInfo.maxDepthBounds = 1.0;

    VkViewport viewport{
            0, 0,
            static_cast<float>(app->swapChainExtent.width),
            static_cast<float>(app->swapChainExtent.height),
            0.0, 1.0
    };
    VkRect2D scissor{{0, 0}, app->swapChainExtent};
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    viewportStateCreateInfo.flags
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewport;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    multisampleStateCreateInfo.flags
    multisampleStateCreateInfo.rasterizationSamples = app->msaaSamples;
    multisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;
    multisampleStateCreateInfo.minSampleShading = 0.2;
    multisampleStateCreateInfo.pSampleMask = nullptr;
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;

    std::vector<VkDynamicState> dynamicStates{
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
    };
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
    dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//    dynamicStateCreateInfo.flags
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    pipelineLayoutCreateInfo.flags
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = VK_NULL_HANDLE;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = VK_NULL_HANDLE;

    if (vkCreatePipelineLayout(app->device, &pipelineLayoutCreateInfo, nullptr, &graphicsPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    graphicsPipelineCreateInfo.flags
    graphicsPipelineCreateInfo.pStages = shaderStageCreateInfos.data();
    graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size());
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pTessellationState = &tessellationStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
    graphicsPipelineCreateInfo.renderPass = app->renderPass;
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.layout = graphicsPipelineLayout;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(app->device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr,
                                  &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(app->device, vertShaderModule, nullptr);
    vkDestroyShaderModule(app->device, fragShaderModule, nullptr);
}

void ParticleRenderer::createParticleData() {
    std::default_random_engine randomEngine((unsigned) time(nullptr));
    std::uniform_real_distribution<float> randomDist(0.0, 1.0);

    int width, height;
    glfwGetWindowSize(app->window, &width, &height);

    particles.resize(PARTICLE_COUNT);
    for (auto &particle: particles) {
        float r = 0.25f * sqrt(randomDist(randomEngine));
        float theta = randomDist(randomEngine) * 2 * 3.14159265358979323846;
        float x = r * cos(theta) * height / width;
        float y = r * sin(theta);
        particle.position = glm::vec2(x, y);
        particle.velocity = glm::normalize(glm::vec2(x, y));
        particle.color = glm::vec4(randomDist(randomEngine), randomDist(randomEngine), randomDist(randomEngine), 1.0f);
    }
}

void ParticleRenderer::createUniformBuffers() {
    uniformBuffers.resize(app->MAX_FRAMES_IN_FLIGHT);
    uniformBufferMemories.resize(app->MAX_FRAMES_IN_FLIGHT);
    uniformBufferMemoriesMapped.resize(app->MAX_FRAMES_IN_FLIGHT);

    VkDeviceSize bufferSize = sizeof(ParticleUniformBufferObject);
    for (int i = 0; i < app->MAX_FRAMES_IN_FLIGHT; ++i) {
        app->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          uniformBuffers[i], uniformBufferMemories[i]);
        vkMapMemory(app->device, uniformBufferMemories[i], 0, bufferSize, 0, &uniformBufferMemoriesMapped[i]);
    }
}

void ParticleRenderer::createShaderStorageBuffers() {
    shaderStorageBuffers.resize(app->MAX_FRAMES_IN_FLIGHT);
    shaderStorageBufferMemories.resize(app->MAX_FRAMES_IN_FLIGHT);

    VkDeviceSize bufferSize = sizeof(Particle) * PARTICLE_COUNT;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    app->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      stagingBuffer, stagingBufferMemory);

    void *data;
    vkMapMemory(app->device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, particles.data(), bufferSize);
    vkUnmapMemory(app->device, stagingBufferMemory);

    for (int i = 0; i < shaderStorageBuffers.size(); ++i) {
        app->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
                                      | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                          shaderStorageBuffers[i], shaderStorageBufferMemories[i]);
        app->copyBuffer(stagingBuffer, shaderStorageBuffers[i], bufferSize);
    }

    vkDestroyBuffer(app->device, stagingBuffer, nullptr);
    vkFreeMemory(app->device, stagingBufferMemory, nullptr);
}

void ParticleRenderer::createDescriptorSets() {
    computeDescriptorSets.resize(app->MAX_FRAMES_IN_FLIGHT);
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts(app->MAX_FRAMES_IN_FLIGHT, computeDescriptorSetLayout);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = app->descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts.data();

    vkAllocateDescriptorSets(app->device, &descriptorSetAllocateInfo, computeDescriptorSets.data());

    for (int i = 0; i < computeDescriptorSets.size(); ++i) {
        std::array<VkWriteDescriptorSet, 3> writeDescriptorSets{};

        VkDescriptorBufferInfo bufferInfo{
                uniformBuffers[i],
                0,
                sizeof(ParticleUniformBufferObject)
        };
        VkDescriptorBufferInfo ssboBufferInInfo{
                shaderStorageBuffers[(i - 1 + computeDescriptorSets.size()) % app->MAX_FRAMES_IN_FLIGHT],
                0,
                sizeof(Particle) * PARTICLE_COUNT
        };
        VkDescriptorBufferInfo ssboBufferOutInfo{
                shaderStorageBuffers[i],
                0,
                sizeof(Particle) * PARTICLE_COUNT
        };

        writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[0].dstBinding = 0;
        writeDescriptorSets[0].dstSet = computeDescriptorSets[i];
        writeDescriptorSets[0].dstArrayElement = 0;
        writeDescriptorSets[0].descriptorCount = 1;
        writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSets[0].pBufferInfo = &bufferInfo;

        writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[1].dstBinding = 1;
        writeDescriptorSets[1].dstSet = computeDescriptorSets[i];
        writeDescriptorSets[1].dstArrayElement = 0;
        writeDescriptorSets[1].descriptorCount = 1;
        writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSets[1].pBufferInfo = &ssboBufferInInfo;

        writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSets[2].dstBinding = 2;
        writeDescriptorSets[2].dstSet = computeDescriptorSets[i];
        writeDescriptorSets[2].dstArrayElement = 0;
        writeDescriptorSets[2].descriptorCount = 1;
        writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writeDescriptorSets[2].pBufferInfo = &ssboBufferOutInfo;

        vkUpdateDescriptorSets(app->device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
    }
}

void ParticleRenderer::update(float deltaTime, uint32_t frameNum) {
    ParticleUniformBufferObject ubo{deltaTime};
    memcpy(uniformBufferMemoriesMapped[frameNum], &ubo, sizeof(ubo));
}

void ParticleRenderer::compute(VkCommandBuffer commandBuffer, uint32_t frameNum) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout,
                            0, 1, &computeDescriptorSets[frameNum], 0, nullptr);
    vkCmdDispatch(commandBuffer, ceil(PARTICLE_COUNT / 256.0), 1, 1);
}

void ParticleRenderer::render(VkCommandBuffer commandBuffer, uint32_t frameNum) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport{
            0, 0,
            static_cast<float>(app->swapChainExtent.width),
            static_cast<float>(app->swapChainExtent.height),
            0, 1,
    };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{0, 0, app->swapChainExtent};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkDeviceSize vertexBufferOffsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &shaderStorageBuffers[frameNum], vertexBufferOffsets);
    vkCmdDraw(commandBuffer, PARTICLE_COUNT, 1, 0, 0);
}

void ParticleRenderer::cleanup() {
    vkDestroyPipeline(app->device, computePipeline, nullptr);
    vkDestroyPipelineLayout(app->device, computePipelineLayout, nullptr);
    vkDestroyDescriptorSetLayout(app->device, computeDescriptorSetLayout, nullptr);

    vkDestroyPipeline(app->device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(app->device, graphicsPipelineLayout, nullptr);

    for (int i = 0; i < app->MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroyBuffer(app->device, uniformBuffers[i], nullptr);
        vkFreeMemory(app->device, uniformBufferMemories[i], nullptr);

        vkDestroyBuffer(app->device, shaderStorageBuffers[i], nullptr);
        vkFreeMemory(app->device, shaderStorageBufferMemories[i], nullptr);
    }
}
