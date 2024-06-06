#include "example_basic_pipeline.h"

namespace polyp {
namespace example {

using namespace polyp::vulkan;

ExampleBasicPipeline::ExampleBasicPipeline() : ExampleBase()
{
    std::vector<RHIContext::CreateInfo::Queue> queInfos{
        {1, vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eTransfer, true}
    };

    mContextInfo.device.queues = queInfos;
}

ExampleBasicPipeline::ModelsData  ExampleBasicPipeline::loadModel()
{
    std::vector<Vertex> vertexData = {
        { {  0.6f,  0.6f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { -0.6f,  0.6f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  0.0f, -0.6f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
    };

    std::vector<uint32_t> indexData = { 0, 1, 2 };

    return std::make_tuple(std::move(vertexData), std::move(indexData));
}

ExampleBasicPipeline::UniformData  ExampleBasicPipeline::loadUniformData()
{
    return UniformData {
        glm::mat4(1.0f),
        glm::mat4(1.0f),
        glm::mat4(1.0f)
    };
}

void ExampleBasicPipeline::createTransferCmd()
{
    auto& ctx    = RHIContext::get();
    auto& device = ctx.device();

    auto familyIdx = ctx.queueFamily(mContextInfo.device.queues[0].flags);

    vk::CommandPoolCreateInfo cmdPoolCreateInfo{};
    cmdPoolCreateInfo.queueFamilyIndex = familyIdx;
    cmdPoolCreateInfo.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    mTransferCmdPool = ctx.device().createCommandPool(cmdPoolCreateInfo);
    if (*mTransferCmdPool == VK_NULL_HANDLE) {
        POLYPFATAL("Failed to create command pool");
    }
    
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool        = *mCmdPool;
    allocInfo.level              = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = 1;
    
    auto cmds = device.allocateCommandBuffers(allocInfo);
    if (cmds.empty() && *cmds[0] != VK_NULL_HANDLE) {
        POLYPFATAL("Failed to allocate command buffers.");
    }
    
    mTransferCmd = std::move(cmds[0]);
    POLYPDEBUG("Primary command buffer created successfully");
}

void ExampleBasicPipeline::createBuffers()
{
    const uint32_t vertexBufferSize  = mVertexData.size() * sizeof(decltype(mVertexData)::value_type);
    const uint32_t indexBufferSize   = mIndexData.size()  * sizeof(decltype(mIndexData)::value_type);
    const uint32_t uniformBufferSize = sizeof(decltype(mUniformData));

    auto vertexUploadBuffer  = utils::createUploadBuffer(vertexBufferSize);
    auto indexUploadBuffer   = utils::createUploadBuffer(indexBufferSize);
    auto uniformUploadBuffer = utils::createUploadBuffer(uniformBufferSize);

    vertexUploadBuffer.fill(mVertexData);
    indexUploadBuffer.fill(mIndexData);
    uniformUploadBuffer.fill((void*)&mUniformData, uniformBufferSize);

    const auto vertUsage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
    const auto indUsage  = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
    const auto sdrUsage  = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer;

    mVertexBuffer  = utils::createDeviceBuffer(vertexBufferSize, vertUsage);
    mIndexBuffer   = utils::createDeviceBuffer(indexBufferSize,  indUsage);
    mUniformBuffer = utils::createDeviceBuffer(uniformBufferSize, sdrUsage);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = CommandBufferUsageFlagBits::eOneTimeSubmit;
    mTransferCmd.begin(beginInfo);

    vk::BufferMemoryBarrier defaultBarrier{};
    defaultBarrier.srcAccessMask       = vk::AccessFlagBits::eNone;
    defaultBarrier.dstAccessMask       = vk::AccessFlagBits::eTransferWrite;
    defaultBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    defaultBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    defaultBarrier.offset              = 0;
    defaultBarrier.size                = VK_WHOLE_SIZE;

    std::array<vk::BufferMemoryBarrier, 3> barriers{};

    barriers[0] = defaultBarrier;
    barriers[1] = defaultBarrier;
    barriers[2] = defaultBarrier;

    barriers[0].buffer = *mVertexBuffer;
    barriers[1].buffer = *mIndexBuffer;
    barriers[2].buffer = *mUniformBuffer;

    mTransferCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlagBits{}, {}, barriers, {});

    vk::BufferCopy copyRegion{ 0,0, vertexBufferSize };
    mTransferCmd.copyBuffer(*vertexUploadBuffer, *mVertexBuffer, { copyRegion });

    copyRegion.size = indexBufferSize;
    mTransferCmd.copyBuffer(*indexUploadBuffer, *mIndexBuffer, { copyRegion });

    copyRegion.size = uniformBufferSize;
    mTransferCmd.copyBuffer(*uniformUploadBuffer, *mUniformBuffer, { copyRegion });

    defaultBarrier.srcAccessMask = defaultBarrier.dstAccessMask;
    defaultBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;

    barriers[0] = defaultBarrier;
    barriers[1] = defaultBarrier;
    barriers[2] = defaultBarrier;

    barriers[0].buffer = *mVertexBuffer;
    barriers[1].buffer = *mIndexBuffer;
    barriers[2].buffer = *mUniformBuffer;

    mTransferCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eVertexInput, vk::DependencyFlagBits{}, {}, barriers, {});

    mTransferCmd.end();
   
    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &*mTransferCmd;
    mQueue.submit(submitInfo);

    RHIContext::get().device().waitIdle();
}

void ExampleBasicPipeline::createLayouts()
{
    auto& device = RHIContext::get().device();
    
    vk::DescriptorSetLayoutBinding layoutBindingInfo{}; // uniform buffer for vertex shader
    layoutBindingInfo.descriptorType  = vk::DescriptorType::eUniformBuffer;
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

void ExampleBasicPipeline::createDS()
{
    auto& device = RHIContext::get().device();

    vk::DescriptorPoolSize descriptorPoolSize;
    descriptorPoolSize.type            = vk::DescriptorType::eUniformBuffer;
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
    dsBufferInfo.range  = sizeof(UniformData);

    vk::WriteDescriptorSet writeDescriptorSet{};

    writeDescriptorSet.dstSet          = *mDescriptorSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType  = vk::DescriptorType::eUniformBuffer;
    writeDescriptorSet.pBufferInfo     = &dsBufferInfo;
    writeDescriptorSet.dstBinding      = 0;

    device.updateDescriptorSets({ writeDescriptorSet }, {});
}

void ExampleBasicPipeline::createPipeline()
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

bool ExampleBasicPipeline::onInit(const WindowInitializedEventArgs& args)
{
    if (!ExampleBase::onInit(args))
        return false;

    mUniformData                      = loadUniformData();
    std::tie(mVertexData, mIndexData) = loadModel();

    try
    {
        createTransferCmd();
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
        POLYPFATAL("Internal fatal error.");
        return false;
    }

    POLYPDEBUG("Initialization finished");

    return true;
}

void ExampleBasicPipeline::draw()
{
    if (mPauseDrawing)
        return;

    POLYPDEBUG(__FUNCTION__);

    acquireSwapChainImage();

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    mCmdBuffer.begin(beginInfo);

    render();

    mCmdBuffer.end();

    present();

    RHIContext::get().device().waitIdle();
}

void ExampleBasicPipeline::render()
{
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

    mCmdBuffer.beginRenderPass(renderPassBeginInfo, SubpassContents::eInline);

    vk::Viewport viewport{};
    viewport.height   = (float)height;
    viewport.width    = (float)width;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    
    std::vector<vk::Viewport> viewpors{ viewport };
    mCmdBuffer.setViewport(0, viewpors);
    
    vk::Rect2D scissor{};
    scissor.extent.width  = width;
    scissor.extent.height = height;
    scissor.offset.x      = 0;
    scissor.offset.y      = 0;
    
    std::vector<vk::Rect2D> scissors{ scissor };
    mCmdBuffer.setScissor(0, scissors);
    
    mCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *mPipelineLayout, 0, { *mDescriptorSet }, {});
    mCmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *mPipeline);
    
    VkDeviceSize offset = { 0 };
    mCmdBuffer.bindVertexBuffers(0, { *mVertexBuffer }, { offset });
    mCmdBuffer.bindIndexBuffer(*mIndexBuffer, 0, vk::IndexType::eUint32);
    mCmdBuffer.drawIndexed(mIndexData.size(), 1, 0, 0, 1);
    mCmdBuffer.endRenderPass();
}

} // example
} // polyp
