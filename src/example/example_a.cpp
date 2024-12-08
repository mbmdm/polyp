#include "example_a.h"

namespace polyp {
namespace vulkan {
namespace example {

bool ExampleA::postInit()
{
    mTransferCmd = utils::createCommandBuffer(mCmdPool, vk::CommandBufferLevel::ePrimary);
    if (*mTransferCmd == VK_NULL_HANDLE) {
        return false;
    }

    POLYPDEBUG("Primary command buffers created successfully");

    std::tie(mVertexData, mIndexData) = loadModel();

    try
    {
        createBuffers();
        createLayouts();
        createDS();
        createPipeline();
    }
    catch (const SystemError& err)
    {
        POLYPFATAL("Exception %d (%s), message %s", err.code().value(), err.code().message().c_str(), err.what());
        return false;
    }
    catch (...)
    {
        POLYPFATAL("Internal fatal error during initialization.");
        return false;
    }

    POLYPDEBUG("Initialization finished");

    mCamera.speed(mCamera.speed() * 5);

    return true;
}

bool ExampleA::postResize()
{
    const auto& gpu     = RHIContext::get().gpu();
    const auto& surface = RHIContext::get().surface();
    const auto& device  = RHIContext::get().device();

    auto [image, view]  = utils::createDepthStencil();
    if (*image == VK_NULL_HANDLE  || *view == VK_NULL_HANDLE)
    {
        POLYPERROR("Internal error: failed to create depth resources.");
        return false;
    }

    mDepthStencil.image = std::move(image);
    mDepthStencil.view  = std::move(view);

    mRenderPass = utils::createRenderPass();
    if (*mRenderPass == VK_NULL_HANDLE)
    {
        POLYPERROR("Internal error: failed to create render pass.");
        return false;
    }

    auto capabilities = gpu.getSurfaceCapabilitiesKHR(*surface);

    if (mSwapChainVeiews.empty())
    {
        POLYPERROR("Internal error: there are no swapchain imave view.");
        return false;
    }

    mFrameBuffers.clear();

    for (auto i = 0; i < mSwapChainVeiews.size(); ++i)
    {
        std::array<vk::ImageView, 2> attachments;
        attachments[0] = *mSwapChainVeiews[i];
        attachments[1] = *mDepthStencil.view;

        vk::FramebufferCreateInfo fbCreateInfo{};
        fbCreateInfo.renderPass      = *mRenderPass;
        fbCreateInfo.attachmentCount = attachments.size();
        fbCreateInfo.pAttachments    = attachments.data();
        fbCreateInfo.width           = capabilities.currentExtent.width;
        fbCreateInfo.height          = capabilities.currentExtent.height;
        fbCreateInfo.layers          = 1;

        auto fb = device.createFramebuffer(fbCreateInfo);
        if (*fb == VK_NULL_HANDLE)
        {
            POLYPERROR("Internal error: failed to create frame buffer.");
            return false;
        }

        mFrameBuffers.push_back(std::move(fb));
    }

    return true;
}

void ExampleA::draw()
{
    prepareDrawCommands();

    updateUniformBuffer();
}

RHIContext::CreateInfo ExampleA::getRHICreateInfo()
{
    auto info = utils::getCreateInfo<RHIContext::CreateInfo>();

    std::vector<RHIContext::CreateInfo::Queue> queInfos {
        {1, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer, true}
    };

    info.device.queues = queInfos;

    return info;
}

void ExampleA::createBuffers()
{
    auto mvpData = getMVP();

    const auto cubeCount = 10;

    const VkDeviceSize vertexBufferSize  = mVertexData.size() * sizeof(decltype(mVertexData)::value_type) * cubeCount;
    const VkDeviceSize indexBufferSize   = mIndexData.size()  * sizeof(decltype(mIndexData)::value_type);
    const VkDeviceSize uniformBufferSize = sizeof(mvpData) * mSwapChainImages.size();

    const auto vertUsage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    const auto indUsage  = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    const auto uplUsage  = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer;

    VkMemoryPropertyFlags uniformMemFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    auto vertexUploadBuffer  = utils::createUploadBuffer(vertexBufferSize);
    auto indexUploadBuffer   = utils::createUploadBuffer(indexBufferSize);
    auto uniformUploadBuffer = utils::createUploadBuffer(uniformBufferSize, uplUsage, uniformMemFlags);

    if (*vertexUploadBuffer  == VK_NULL_HANDLE ||
        *indexUploadBuffer   == VK_NULL_HANDLE ||
        *uniformUploadBuffer == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to create upload buffers.");
    }

    vertexUploadBuffer.fill(mVertexData);
    indexUploadBuffer.fill(mIndexData);
    uniformUploadBuffer.fill((void*)&mvpData, uniformBufferSize);

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    for (size_t i = 0; i < cubeCount; ++i)
    {
        uint32_t offset = (mVertexData.size() * sizeof(decltype(mVertexData)::value_type)) * i;

        auto cpVertexData = mVertexData;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        float angle = 20.0f * i;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

        for (size_t v = 0; v < cpVertexData.size(); v++)
        {
            auto& vertex = cpVertexData[v];
            glm::vec4 transformedVertex{ vertex.position[0], vertex.position[1], vertex.position[2], 1.0 };
            transformedVertex = model * transformedVertex;

            vertex.position[0] = transformedVertex.x;
            vertex.position[1] = transformedVertex.y;
            vertex.position[2] = transformedVertex.z;
        }

        vertexUploadBuffer.fill(cpVertexData, offset);
    }

    mVertexBuffer  = utils::createDeviceBuffer(vertexBufferSize, vertUsage);
    mIndexBuffer   = utils::createDeviceBuffer(indexBufferSize,  indUsage);
    mUniformBuffer = std::move(uniformUploadBuffer);

    if (*mVertexBuffer  == VK_NULL_HANDLE ||
        *mIndexBuffer   == VK_NULL_HANDLE ||
        *mUniformBuffer == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to create device buffers.");
    }

    vk::BufferMemoryBarrier barrier{};
    barrier.srcAccessMask       = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask       = vk::AccessFlagBits::eMemoryRead;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.offset              = 0;
    barrier.size                = VK_WHOLE_SIZE;

    std::array<vk::BufferMemoryBarrier, 2> barriers{};

    barriers[0] = barrier;
    barriers[1] = barrier;

    barriers[0].buffer = *mVertexBuffer;
    barriers[1].buffer = *mIndexBuffer;

    mTransferCmd.reset();

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    mTransferCmd.begin(beginInfo);

    vk::BufferCopy copyRegion{ 0, 0, vertexBufferSize };
    mTransferCmd.copyBuffer(*vertexUploadBuffer, *mVertexBuffer, { copyRegion });

    copyRegion.size = indexBufferSize;
    mTransferCmd.copyBuffer(*indexUploadBuffer, *mIndexBuffer, { copyRegion });

    // The barriers are useless because of queue idle (added for demonstration)
    mTransferCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eVertexInput, vk::DependencyFlagBits{}, {}, barriers, {});

    barrier.buffer        = *mUniformBuffer;
    barrier.srcAccessMask = vk::AccessFlagBits::eHostWrite;

    mTransferCmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eVertexInput, vk::DependencyFlagBits{}, {}, { barrier }, {});

    mTransferCmd.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &*mTransferCmd;

    mQueue.submit(submitInfo);
    mQueue.waitIdle();
}

void ExampleA::createLayouts()
{
    auto& device = RHIContext::get().device();
    
    vk::DescriptorSetLayoutBinding layoutBindingInfo{}; // uniform buffer for vertex shader
    layoutBindingInfo.descriptorType  = vk::DescriptorType::eUniformBufferDynamic;
    layoutBindingInfo.descriptorCount = 1;
    layoutBindingInfo.stageFlags      = vk::ShaderStageFlagBits::eVertex;
    
    vk::DescriptorSetLayoutCreateInfo dsLayoutCreateInfo{};
    dsLayoutCreateInfo.bindingCount = 1;
    dsLayoutCreateInfo.pBindings    = &layoutBindingInfo;
    
    mDSLayout = device.createDescriptorSetLayout(dsLayoutCreateInfo);
    
    vk::PipelineLayoutCreateInfo pipeLayoutCreateInfo{};
    pipeLayoutCreateInfo.setLayoutCount = 1;
    pipeLayoutCreateInfo.pSetLayouts    = &*mDSLayout;
    
    mPipelineLayout = device.createPipelineLayout(pipeLayoutCreateInfo);
}

void ExampleA::createDS()
{
    auto& device = RHIContext::get().device();

    vk::DescriptorPoolSize descriptorPoolSize;
    descriptorPoolSize.type            = vk::DescriptorType::eUniformBufferDynamic;
    descriptorPoolSize.descriptorCount = 1;

    vk::DescriptorPoolCreateInfo dsPoolCreateInfo{};
    dsPoolCreateInfo.poolSizeCount = 1;
    dsPoolCreateInfo.pPoolSizes    = &descriptorPoolSize;
    dsPoolCreateInfo.maxSets       = 1;
    dsPoolCreateInfo.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

    mDesriptorPool = device.createDescriptorPool(dsPoolCreateInfo);
    
    vk::DescriptorSetAllocateInfo dsAllocInfo{};
    dsAllocInfo.descriptorPool     = *mDesriptorPool;
    dsAllocInfo.descriptorSetCount = 1;
    dsAllocInfo.pSetLayouts        = &*mDSLayout;

    auto sets = device.allocateDescriptorSets(dsAllocInfo);
    POLYPASSERT(!sets.empty());

    mDescriptorSet = std::move(sets[0]);

    vk::DescriptorBufferInfo dsBufferInfo{};
    dsBufferInfo.buffer = *mUniformBuffer;
    dsBufferInfo.range = static_cast<uint32_t>(sizeof(MVP));

    vk::WriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.dstSet          = *mDescriptorSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType  = vk::DescriptorType::eUniformBufferDynamic;
    writeDescriptorSet.pBufferInfo     = &dsBufferInfo;
    writeDescriptorSet.dstBinding      = 0;

    device.updateDescriptorSets({ writeDescriptorSet }, {});
}

void ExampleA::createPipeline()
{
    vk::GraphicsPipelineCreateInfo pipeCreateInfo;
    pipeCreateInfo.layout     = *mPipelineLayout;
    pipeCreateInfo.renderPass = *mRenderPass;

    // input assembly state
    vk::PipelineInputAssemblyStateCreateInfo inputAsmStateCreateInfo{};
    inputAsmStateCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;

    // Rasterization state
    vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo;
    rasterizationStateCreateInfo.polygonMode             = vk::PolygonMode::eFill; 
    rasterizationStateCreateInfo.cullMode                = vk::CullModeFlagBits::eNone;
    rasterizationStateCreateInfo.frontFace               = vk::FrontFace::eCounterClockwise;
    rasterizationStateCreateInfo.depthClampEnable        = vk::False;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = vk::False;
    rasterizationStateCreateInfo.depthBiasEnable         = vk::False;
    rasterizationStateCreateInfo.lineWidth               = 1.0f;

    // Blend state
    vk::PipelineColorBlendAttachmentState blendAttachmentState{};
    blendAttachmentState.colorWriteMask = ColorComponentFlagBits::eA | ColorComponentFlagBits::eR | 
                                          ColorComponentFlagBits::eG | ColorComponentFlagBits::eB;
    blendAttachmentState.blendEnable    = vk::False;

    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments    = &blendAttachmentState;

    // Viewport state
    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount  = 1;

    // Enable dynamic states
    std::vector<vk::DynamicState> dynamicStateEnables{ DynamicState::eViewport, DynamicState::eScissor };

    vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    dynamicStateCreateInfo.pDynamicStates    = dynamicStateEnables.data();
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

    // Depth and stencil state
    vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;
    depthStencilStateCreateInfo.depthTestEnable       = vk::True;
    depthStencilStateCreateInfo.depthWriteEnable      = vk::True;
    depthStencilStateCreateInfo.depthCompareOp        = vk::CompareOp::eLessOrEqual; /*VK_COMPARE_OP_LESS_OR_EQUAL*/;
    depthStencilStateCreateInfo.depthBoundsTestEnable = vk::False;
    depthStencilStateCreateInfo.stencilTestEnable     = vk::False;
    depthStencilStateCreateInfo.back.setFailOp(vk::StencilOp::eKeep);
    depthStencilStateCreateInfo.back.setPassOp(vk::StencilOp::eKeep);
    depthStencilStateCreateInfo.back.setCompareOp(vk::CompareOp::eAlways);
    depthStencilStateCreateInfo.front = depthStencilStateCreateInfo.back;

    // Multi sampling state
    vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo;
    multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;

    // Vertex input format for a pipeline
    vk::VertexInputBindingDescription vertexInputBinding{};
    vertexInputBinding.binding   = 0;
    vertexInputBinding.stride    = sizeof(Vertex);
    vertexInputBinding.inputRate = vk::VertexInputRate::eVertex;

    std::array<vk::VertexInputAttributeDescription, 2> vertexInputAttributs;
    vertexInputAttributs[0].binding  = 0;
    vertexInputAttributs[0].location = 0;
    vertexInputAttributs[0].format   = vk::Format::eR32G32B32Sfloat;
    vertexInputAttributs[0].offset   = offsetof(Vertex, position);
    vertexInputAttributs[1].binding  = 0;
    vertexInputAttributs[1].location = 1;
    vertexInputAttributs[1].format   = vk::Format::eR32G32B32Sfloat;
    vertexInputAttributs[1].offset   = offsetof(Vertex, color);

    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.vertexBindingDescriptionCount   = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions      = &vertexInputBinding;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 2;
    vertexInputStateCreateInfo.pVertexAttributeDescriptions    = vertexInputAttributs.data();

    // Shaders
    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{};

    auto [vertexShader, indexShader] = loadShaders();

    shaderStages[0].stage  = vk::ShaderStageFlagBits::eVertex;
    shaderStages[0].module = *vertexShader;
    shaderStages[0].pName  = "main";

    shaderStages[1].stage  = vk::ShaderStageFlagBits::eFragment;
    shaderStages[1].module = *indexShader;
    shaderStages[1].pName  = "main";

    pipeCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipeCreateInfo.pStages    = shaderStages.data();

    pipeCreateInfo.pVertexInputState   = &vertexInputStateCreateInfo;
    pipeCreateInfo.pInputAssemblyState = &inputAsmStateCreateInfo;
    pipeCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipeCreateInfo.pColorBlendState    = &colorBlendStateCreateInfo;
    pipeCreateInfo.pMultisampleState   = &multisampleStateCreateInfo;
    pipeCreateInfo.pViewportState      = &viewportStateCreateInfo;
    pipeCreateInfo.pDepthStencilState  = &depthStencilStateCreateInfo;
    pipeCreateInfo.pDynamicState       = &dynamicStateCreateInfo;

    mPipeline = RHIContext::get().device().createGraphicsPipeline(VK_NULL_HANDLE, pipeCreateInfo);
}

void ExampleA::updateUniformBuffer()
{
    const auto mvpData = getMVP();

    auto pos = (mCurrSwImIndex + 1) % mSwapChainImages.size();

    mUniformBuffer.fill((void*)&mvpData, sizeof(mvpData), sizeof(MVP) * pos);
}

void ExampleA::prepareDrawCommands()
{
    CommandBuffer& cmd = mDrawCmds[mCurrSwImIndex];

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    cmd.begin(beginInfo);

    vk::BufferMemoryBarrier barrier{};
    barrier.srcAccessMask       = vk::AccessFlagBits::eHostWrite;
    barrier.dstAccessMask       = vk::AccessFlagBits::eMemoryRead;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.offset              = 0;
    barrier.size                = VK_WHOLE_SIZE;
    barrier.buffer              = *mUniformBuffer;

    std::array<vk::BufferMemoryBarrier, 1> barriers{ barrier };

    //cmd.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eVertexInput, vk::DependencyFlagBits{}, {}, barriers, {});

    const auto& ctx = RHIContext::get();

    auto capabilities = ctx.gpu().getSurfaceCapabilitiesKHR(*ctx.surface());

    const uint32_t width  = capabilities.currentExtent.width;
    const uint32_t height = capabilities.currentExtent.height;

    vk::ClearValue clearValues[2];
    clearValues[0].color        = vk::ClearColorValue{ 0.4f, 0.4f, 0.4f, 1.0f };
    clearValues[1].depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };

    vk::RenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.renderPass               = *mRenderPass;
    renderPassBeginInfo.renderArea.offset.x      = 0;
    renderPassBeginInfo.renderArea.offset.y      = 0;
    renderPassBeginInfo.renderArea.extent.width  = width;
    renderPassBeginInfo.renderArea.extent.height = height;
    renderPassBeginInfo.clearValueCount          = 2;
    renderPassBeginInfo.pClearValues             = clearValues;
    renderPassBeginInfo.framebuffer              = *mFrameBuffers[mCurrSwImIndex];

    cmd.beginRenderPass(renderPassBeginInfo, SubpassContents::eInline);

    vk::Viewport viewport{};
    viewport.height   = (float)height;
    viewport.width    = (float)width;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;

    // View port transformation flipping (like in OpenGL)
    //viewport.height = -(float)height;
    //viewport.width  = (float)width;
    //viewport.x      = 0;
    //viewport.y      = height;

    std::vector<vk::Viewport> viewpors{ viewport };
    cmd.setViewport(0, viewpors);

    vk::Rect2D scissor{};
    scissor.extent.width = width;
    scissor.extent.height = height;
    scissor.offset.x      = 0;
    scissor.offset.y      = 0;

    std::vector<vk::Rect2D> scissors{ scissor };
    cmd.setScissor(0, scissors);

    std::vector<uint32_t> dynamicOffsets{ static_cast<uint32_t>(sizeof(MVP)) * mCurrSwImIndex };

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *mPipelineLayout, 0, { *mDescriptorSet }, dynamicOffsets);
    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, *mPipeline);

    const auto cubeCount = 10;

    cmd.bindIndexBuffer(*mIndexBuffer, 0, vk::IndexType::eUint32);

    for (size_t i = 0; i < cubeCount; i++)
    {
        VkDeviceSize offset = (mVertexData.size() * sizeof(decltype(mVertexData)::value_type)) * i;
        cmd.bindVertexBuffers(0, { *mVertexBuffer }, { offset });
        cmd.drawIndexed(mIndexData.size(), 1, 0, 0, 1);
    }

    //VkDeviceSize offset = { 0 };
    //cmd.bindVertexBuffers(0, { *mVertexBuffer }, { offset });
    //cmd.bindIndexBuffer(*mIndexBuffer, 0, vk::IndexType::eUint32);
    //cmd.drawIndexed(mIndexData.size(), 1, 0, 0, 1);
    cmd.endRenderPass();

    cmd.end();
}

} // example
} // vulkan
} // polyp
