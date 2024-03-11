#include <common.h>
#include <example.h>
#include <constants.h>
#include <destroyable_handle.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <fstream>
#include <memory>

#include "vulkan/vulkan_raii.hpp"

using namespace polyp;
using namespace polyp::vk;
using namespace polyp::tools;

namespace {

struct Vertex {
    float position[3];
    float color[3];
};

struct ShaderData {
    glm::mat4 projectionMatrix;
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
};

auto createVertexBuffer(Device::ConstPtr device)
{
    using namespace polyp::vk::utils;

    auto vk  = device->vk();
    auto dev = device->native();
    auto gpu = device->gpu();
    auto que = device->queue(VK_QUEUE_GRAPHICS_BIT);

    void* mapped = nullptr;

    DESTROYABLE(VkCommandBuffer) cmd 
        = { device->cmdBuffer(que, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY), device };

    std::vector<Vertex> vertexBuffer{
        { {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { -1.0f,  1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { {  0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } }
    };
    uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

    std::vector<uint32_t> indexBuffer{ 0, 1, 2 };
    uint32_t indexBufferSize = static_cast<uint32_t>(indexBuffer.size()) * sizeof(uint32_t);

    // Vertex buffer
    BufferResource vertexBufferUpload 
        = createBuffer(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vertexBufferSize,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    BufferResource vertexBufferGpu    
        = createBuffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       vertexBufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CHECKRET(vk.MapMemory(dev, *vertexBufferUpload.memory, 0, VK_WHOLE_SIZE, 0, (void**)&mapped));
    memcpy(mapped, vertexBuffer.data(), vertexBufferSize);
    vk.UnmapMemory(dev, *vertexBufferUpload.memory);
    
    // Index buffer
    BufferResource indexBufferUpload 
        = createBuffer(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, indexBufferSize,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    BufferResource indexBufferGpu    
        = createBuffer(device, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       indexBufferSize, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CHECKRET(vk.MapMemory(dev, *indexBufferUpload.memory, 0, VK_WHOLE_SIZE, 0, (void**)&mapped));
    memcpy(mapped, indexBuffer.data(), indexBufferSize);
    vk.UnmapMemory(dev, *indexBufferUpload.memory);

    VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    CHECKRET(vk.BeginCommandBuffer(*cmd, &beginInfo));

    VkBufferCopy copyRegion{ 0,0, vertexBufferSize };
    vk.CmdCopyBuffer(*cmd, *vertexBufferUpload.buffer, *vertexBufferGpu.buffer, 1, &copyRegion);

    copyRegion.size = indexBufferSize;
    vk.CmdCopyBuffer(*cmd, *indexBufferUpload.buffer, *indexBufferGpu.buffer, 1, &copyRegion);

    CHECKRET(vk.EndCommandBuffer(*cmd));
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = cmd.pNative();
    
    CHECKRET(vk.QueueSubmit(que, 1, &submitInfo, VK_NULL_HANDLE));
    CHECKRET(vk.DeviceWaitIdle(dev));
    CHECKRET(vk.ResetCommandBuffer(*cmd, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));
  
    return std::make_tuple(std::move(vertexBufferGpu), std::move(indexBufferGpu));
}

auto createDsLayout(Device::ConstPtr device) {
    auto vk = device->vk();
    auto dev = device->native();

    VkDescriptorSetLayoutBinding layoutBindingInfo{}; // binding 0 aka Uniform Buffer for vertex shader
    layoutBindingInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindingInfo.descriptorCount = 1;
    layoutBindingInfo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBindingInfo.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo dsLayoutCreateInfo{};
    dsLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dsLayoutCreateInfo.pNext = nullptr;
    dsLayoutCreateInfo.bindingCount = 1;
    dsLayoutCreateInfo.pBindings = &layoutBindingInfo;

    VkDescriptorSetLayout dsLayout = VK_NULL_HANDLE;
    CHECKRET(vk.CreateDescriptorSetLayout(dev, &dsLayoutCreateInfo, nullptr, &dsLayout));

    return DESTROYABLE(VkDescriptorSetLayout){ dsLayout, device };
}

auto createPipeLayout(Device::ConstPtr device, VkDescriptorSetLayout dsLayout) {
    auto vk = device->vk();
    auto dev = device->native();

    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.setLayoutCount = 1;
    createInfo.pSetLayouts = &dsLayout;

    VkPipelineLayout pipeLayout = VK_NULL_HANDLE;
    CHECKRET(vk.CreatePipelineLayout(dev, &createInfo, nullptr, &pipeLayout));

    return DESTROYABLE(VkPipelineLayout) { pipeLayout, device };
}

auto createDescriptorPool(Device::ConstPtr device) {
    auto vk = device->vk();
    auto dev = device->native();

    VkDescriptorPoolSize descriptorTypeCounts[1];
    descriptorTypeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorTypeCounts[0].descriptorCount = 1;

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.poolSizeCount = 1;
    createInfo.pPoolSizes = descriptorTypeCounts;
    createInfo.maxSets = 1;

    VkDescriptorPool descrPool = VK_NULL_HANDLE;
    CHECKRET(vk.CreateDescriptorPool(dev, &createInfo, nullptr, &descrPool));

    return DESTROYABLE(VkDescriptorPool){ descrPool, device };
}

auto createDescriptorSets(Device::ConstPtr device, VkDescriptorPool pool, VkDescriptorSetLayout layout) {
    auto vk = device->vk();
    auto dev = device->native();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    VkDescriptorSet dsSet = VK_NULL_HANDLE;
    CHECKRET(vk.AllocateDescriptorSets(dev, &allocInfo, &dsSet));

    return DESTROYABLE(VkDescriptorSet){ dsSet, device };
}

auto updateDescriptorSets(Device::ConstPtr device, VkDescriptorSet dsSet, VkBuffer uniformBuffer) {
    auto vk = device->vk();
    auto dev = device->native();

    VkWriteDescriptorSet writeDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

    // The buffer's information is passed using a descriptor info structure
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = uniformBuffer;
    bufferInfo.range = sizeof(ShaderData);

    // Binding 0 : Uniform buffer
    writeDescriptorSet.dstSet = dsSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &bufferInfo;
    writeDescriptorSet.dstBinding = 0;

    vk.UpdateDescriptorSets(dev, 1, &writeDescriptorSet, 0, nullptr);
}

auto createPipeline(Device::ConstPtr device, VkPipelineLayout layout, VkRenderPass renderPass) {
    auto vk = device->vk();
    auto dev = device->native();

    VkGraphicsPipelineCreateInfo pipeCreateInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipeCreateInfo.layout     = layout;
    pipeCreateInfo.renderPass = renderPass;

    // input assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAsmStateCreateInfo
        { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAsmStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo
        { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizationStateCreateInfo.polygonMode             = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.cullMode                = VK_CULL_MODE_NONE;
    rasterizationStateCreateInfo.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo.depthClampEnable        = VK_FALSE;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasEnable         = VK_FALSE;
    rasterizationStateCreateInfo.lineWidth               = 1.0f;

    // Blend state
    VkPipelineColorBlendAttachmentState blendAttachmentState{};
    blendAttachmentState.colorWriteMask = 0xf;
    blendAttachmentState.blendEnable    = VK_FALSE;
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments    = &blendAttachmentState;

    // Viewport state
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.scissorCount = 1;

    // Enable dynamic states
    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo
        { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamicStateCreateInfo.pDynamicStates = dynamicStateEnables.data();
    dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

    // Depth and stencil state
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo
        { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    depthStencilStateCreateInfo.front = depthStencilStateCreateInfo.back;

    // Multi sampling state
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo
        { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCreateInfo.pSampleMask = nullptr;

    // Vertex input format for a pipeline
    VkVertexInputBindingDescription vertexInputBinding{};
    vertexInputBinding.binding = 0;
    vertexInputBinding.stride = sizeof(Vertex);
    vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributs;
    vertexInputAttributs[0].binding = 0;
    vertexInputAttributs[0].location = 0;
    vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributs[0].offset = offsetof(Vertex, position);
    vertexInputAttributs[1].binding = 0;
    vertexInputAttributs[1].location = 1;
    vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributs[1].offset = offsetof(Vertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBinding;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 2;
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributs.data();

    // Shaders
    auto loadSPIRV = [&device](std::string path) {
        std::ifstream is(path, std::ios::binary | std::ios::in | std::ios::ate);

        size_t size = 0;
        std::unique_ptr<char> code{ nullptr };

        if (is.is_open())
        {
            size = is.tellg();
            is.seekg(0, std::ios::beg);
            code.reset(new char[size]);
            is.read(code.get(), size);
            is.close();
            POLYPASSERT(size > 0 && "Load shared SPIR-V failed");
        }

        if (!code) {
            POLYPFATAL("Failed to load SPIR-V file");
        }

        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = size;
        createInfo.pCode = (uint32_t*)code.get();

        VkShaderModule shaderModule = VK_NULL_HANDLE;
        CHECKRET(device->vk().CreateShaderModule(device->native(), &createInfo, nullptr, &shaderModule));

        POLYPASSERTNOTEQUAL(shaderModule, VK_NULL_HANDLE);

        return DESTROYABLE(VkShaderModule) {shaderModule, device};
    };
    
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

    auto vertShader = loadSPIRV("shaders/simple_triangle/simple_triangle.vert.spv");
    auto indxShader = loadSPIRV("shaders/simple_triangle/simple_triangle.frag.spv");

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = *vertShader;
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = *indxShader;
    shaderStages[1].pName = "main";


    pipeCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipeCreateInfo.pStages = shaderStages.data();

    pipeCreateInfo.pVertexInputState   = &vertexInputStateCreateInfo;
    pipeCreateInfo.pInputAssemblyState = &inputAsmStateCreateInfo;
    pipeCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    pipeCreateInfo.pColorBlendState    = &colorBlendStateCreateInfo;
    pipeCreateInfo.pMultisampleState   = &multisampleStateCreateInfo;
    pipeCreateInfo.pViewportState      = &viewportStateCreateInfo;
    pipeCreateInfo.pDepthStencilState  = &depthStencilStateCreateInfo;
    pipeCreateInfo.pDynamicState       = &dynamicStateCreateInfo;

    VkPipeline pipeline = VK_NULL_HANDLE;
    CHECKRET(vk.CreateGraphicsPipelines(dev, VK_NULL_HANDLE, 1, &pipeCreateInfo, nullptr, &pipeline));

    return DESTROYABLE(VkPipeline) { pipeline, device };
}

} // anonimus namespace

class SimpleTriangle : public polyp::vk::example::ExampleBase {
protected:
    DESTROYABLE(VkPipeline)            mPipeline;
    DESTROYABLE(VkPipelineLayout)      mPipelineLayout;
    DESTROYABLE(VkDescriptorSetLayout) mDsLayout;
    DESTROYABLE(VkDescriptorSet)       mDescriptorSet;
    DESTROYABLE(VkDescriptorPool)      mDesriptorPool;
    utils::BufferResource              mIndexBuffer;
    utils::BufferResource              mVertexBuffer;
    utils::BufferResource              mUniformBuffer;
    void* mUniformMapped;

public:
    virtual bool onInit(WindowInstance inst, WindowHandle hwnd) override {
        if (!ExampleBase::onInit(inst, hwnd))
            return false;

        auto vk = mDevice->vk();
        auto dev = mDevice->native();
        auto gpu = mDevice->gpu();

        // create vertex and index buffers
        auto [vertexBuffer, indexBuffer] = createVertexBuffer(mDevice);
        mVertexBuffer.buffer = std::move(vertexBuffer.buffer);
        mVertexBuffer.memory = std::move(vertexBuffer.memory);
        mIndexBuffer.buffer = std::move(indexBuffer.buffer);
        mIndexBuffer.memory = std::move(indexBuffer.memory);

        // create uniform buffer
        mUniformBuffer = utils::createBuffer(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(ShaderData),
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vk.MapMemory(dev, *mUniformBuffer.memory, 0, sizeof(ShaderData), 0, &mUniformMapped);

        // cretae ds and pipeline layouts
        mDsLayout = createDsLayout(mDevice);
        mPipelineLayout = createPipeLayout(mDevice, *mDsLayout);

        // create descriptor
        mDesriptorPool = createDescriptorPool(mDevice);
        mDescriptorSet = createDescriptorSets(mDevice, *mDesriptorPool, *mDsLayout);
        updateDescriptorSets(mDevice, *mDescriptorSet, *mUniformBuffer.buffer);

        // create pipeline
        mPipeline = createPipeline(mDevice, *mPipelineLayout, *mRenderPass);
    }
    virtual void draw() override {
        //preDraw();
        auto [im, imIdx] = mSwapchain->aquireNextImage();
        if (im == VK_NULL_HANDLE) {
            POLYPFATAL("Failed to get Swapchain images");
        }

        currSwImIndex = imIdx;

        VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        CHECKRET(mDevice->vk().BeginCommandBuffer(mCmdBuffer, &beginInfo));

        triangle();

        //postDraw();
        CHECKRET(mDevice->vk().EndCommandBuffer(mCmdBuffer));;

        VkSubmitInfo submitIinfo = {
            VK_STRUCTURE_TYPE_SUBMIT_INFO, // VkStructureType                sType
            nullptr,                       // const void                   * pNext
            0,                             // uint32_t                       waitSemaphoreCount
            nullptr,                       // const VkSemaphore            * pWaitSemaphores
            nullptr,                       // const VkPipelineStageFlags   * pWaitDstStageMask
            1,                             // uint32_t                       commandBufferCount
            &mCmdBuffer,                   // const VkCommandBuffer        * pCommandBuffers
            1,                             // uint32_t                       signalSemaphoreCount
            &mReadyToPresent               // const VkSemaphore            * pSignalSemaphores
        };
        CHECKRET(mDevice->vk().QueueSubmit(mQueue, 1, &submitIinfo, *mSubmitFence));

        VkPresentInfoKHR presentInfo = {
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // VkStructureType          sType
            nullptr,                            // const void*              pNext
            1,                                  // uint32_t                 waitSemaphoreCount
            &mReadyToPresent,                   // const VkSemaphore      * pWaitSemaphores
            1,                                  // uint32_t                 swapchainCount
            mSwapchain->pNative(),              // const VkSwapchainKHR   * pSwapchains
            &currSwImIndex,                     // const uint32_t         * pImageIndices
            nullptr                             // VkResult*                pResults
        };
        CHECKRET(mDevice->vk().QueuePresentKHR(mQueue, &presentInfo));

        CHECKRET(mDevice->vk().WaitForFences(mDevice->native(), 1, &mSubmitFence, VK_TRUE, constants::kFenceTimeout));
        CHECKRET(mDevice->vk().GetFenceStatus(mDevice->native(),   *mSubmitFence));
        CHECKRET(mDevice->vk().ResetFences(mDevice->native(), 1,   &mSubmitFence));
    }
private:
    void triangle() {
        const auto vk = mDevice->vk();
        const auto dev = mDevice->native();
        const auto width = mSwapchain->width();
        const auto height = mSwapchain->height();

        ShaderData shaderData{};
        shaderData.projectionMatrix = glm::mat4(1.0f);
        shaderData.viewMatrix = glm::mat4(1.0f);
        shaderData.modelMatrix = glm::mat4(1.0f);

        memcpy(mUniformMapped, &shaderData, sizeof(ShaderData));

        VkClearValue clearValues[2];
        clearValues[0].color = { { 0.0f, 0.0f, 0.2f, 1.0f } };
        clearValues[1].depthStencil = { 1.0f, 0 };

        VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        renderPassBeginInfo.renderPass = *mRenderPass;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = width;
        renderPassBeginInfo.renderArea.extent.height = height;
        renderPassBeginInfo.clearValueCount = 2;
        renderPassBeginInfo.pClearValues = clearValues;
        renderPassBeginInfo.framebuffer = *mFrameBuffers[currSwImIndex];

        vk.CmdBeginRenderPass(mCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.height = (float)height;
        viewport.width = (float)width;
        viewport.minDepth = (float)0.0f;
        viewport.maxDepth = (float)1.0f;
        vk.CmdSetViewport(mCmdBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent.width = width;
        scissor.extent.height = height;
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vk.CmdSetScissor(mCmdBuffer, 0, 1, &scissor);

        vk.CmdBindDescriptorSets(mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipelineLayout, 0, 1, mDescriptorSet.pNative(), 0, nullptr);
        vk.CmdBindPipeline(mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *mPipeline);

        VkDeviceSize offset = { 0 };
        vk.CmdBindVertexBuffers(mCmdBuffer, 0, 1, mVertexBuffer.buffer.pNative(), &offset);
        vk.CmdBindIndexBuffer(mCmdBuffer, *mIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
        vk.CmdDrawIndexed(mCmdBuffer, 3, 1, 0, 0, 1);

        vk.CmdEndRenderPass(mCmdBuffer);
    }
};

::vk::raii::Instance makeInstance(::vk::raii::Context const& context,
    std::string const& appName,
    std::string const& engineName,
    std::vector<std::string> const& layers = {},
    std::vector<std::string> const& extensions = {},
    uint32_t                         apiVersion = VK_API_VERSION_1_0)
{
    ::vk::ApplicationInfo       applicationInfo(appName.c_str(), 1, engineName.c_str(), 1, apiVersion);
    std::vector<char const*> enabledLayers{};
    std::vector<char const*> enabledExtensions{};

    ::vk::InstanceCreateInfo createInfo{};

    return ::vk::raii::Instance(context, createInfo);
}

int main() {

    ::vk::raii::Context  context;
    ::vk::raii::Instance instance = makeInstance(context, "123", "456", {});

    std::string title{ constants::kWindowTitle };
    title += ": simple triangle";

    tools::IRenderer::Ptr sample = std::make_shared<SimpleTriangle>();
    tools::PolypWindow win{ title.c_str(), 0, 0, 1024, 600, sample };

    win.run();
}