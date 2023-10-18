#include <common.h>
#include <example.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

using namespace polyp;
using namespace polyp::engine;
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
    using namespace polyp::engine::utils;

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
        = createBuffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
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

 //   // Input attribute bindings describe shader attribute locations and memory layouts
 //   std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributs;
 //   // These match the following shader layout (see triangle.vert):
 //   //	layout (location = 0) in vec3 inPos;
 //   //	layout (location = 1) in vec3 inColor;
 //   // Attribute location 0: Position
 //   vertexInputAttributs[0].binding = 0;
 //   vertexInputAttributs[0].location = 0;
 //   // Position attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
 //   vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
 //   vertexInputAttributs[0].offset = offsetof(Vertex, position);
 //   // Attribute location 1: Color
 //   vertexInputAttributs[1].binding = 0;
 //   vertexInputAttributs[1].location = 1;
 //   // Color attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
 //   vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
 //   vertexInputAttributs[1].offset = offsetof(Vertex, color);
 //
 //   // Vertex input state used for pipeline creation
 //   VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
 //   vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
 //   vertexInputStateCI.vertexBindingDescriptionCount = 1;
 //   vertexInputStateCI.pVertexBindingDescriptions = &vertexInputBinding;
 //   vertexInputStateCI.vertexAttributeDescriptionCount = 2;
 //   vertexInputStateCI.pVertexAttributeDescriptions = vertexInputAttributs.data();
 //
 //   // Shaders
 //   std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};
 //
 //   // Vertex shader
 //   shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
 //   // Set pipeline stage for this shader
 //   shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
 //   // Load binary SPIR-V shader
 //   shaderStages[0].module = loadSPIRVShader(getShadersPath() + "triangle/triangle.vert.spv");
 //   // Main entry point for the shader
 //   shaderStages[0].pName = "main";
 //   assert(shaderStages[0].module != VK_NULL_HANDLE);
 //
 //   // Fragment shader
 //   shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
 //   // Set pipeline stage for this shader
 //   shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
 //   // Load binary SPIR-V shader
 //   shaderStages[1].module = loadSPIRVShader(getShadersPath() + "triangle/triangle.frag.spv");
 //   // Main entry point for the shader
 //   shaderStages[1].pName = "main";
 //   assert(shaderStages[1].module != VK_NULL_HANDLE);
 //
 //   // Set pipeline shader stage info
 //   pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
 //   pipelineCI.pStages = shaderStages.data();
 //
 //   // Assign the pipeline states to the pipeline creation info structure
 //   pipelineCI.pVertexInputState = &vertexInputStateCI;
 //   pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
 //   pipelineCI.pRasterizationState = &rasterizationStateCI;
 //   pipelineCI.pColorBlendState = &colorBlendStateCI;
 //   pipelineCI.pMultisampleState = &multisampleStateCI;
 //   pipelineCI.pViewportState = &viewportStateCI;
 //   pipelineCI.pDepthStencilState = &depthStencilStateCI;
 //   pipelineCI.pDynamicState = &dynamicStateCI;
 //
 //   // Create rendering pipeline using the specified states
 //   VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipeline));
 //
 //   // Shader modules are no longer needed once the graphics pipeline has been created
 //   vkDestroyShaderModule(device, shaderStages[0].module, nullptr);
 //   vkDestroyShaderModule(device, shaderStages[1].module, nullptr);
}

} // anonimus namespace

class SimpleTriangle : public engine::example::ExampleBase {
protected:
    DESTROYABLE(VkPipeline)            mPipeline;
    DESTROYABLE(VkPipelineLayout)      mPipelineLayout;
    DESTROYABLE(VkDescriptorSetLayout) mDsLayout;
    DESTROYABLE(VkDescriptorSet)       mDescriptorSet;
    DESTROYABLE(VkDescriptorPool)      mDesriptorPool;
	utils::BufferResource              mIndexBuffer;
	utils::BufferResource              mVertexBuffer;
    utils::BufferResource              mUniformBuffer;
    void*                              mUniformMapped;

public:
    virtual bool onInit(WindowInstance inst, WindowHandle hwnd) override {
        if (!ExampleBase::onInit(inst, hwnd))
            return false;

        auto vk  = mDevice->vk();
        auto dev = mDevice->native();
        auto gpu = mDevice->gpu();

        // create vertex and index buffers
        auto [vertexBuffer, indexBuffer] = createVertexBuffer(mDevice);
        mVertexBuffer.buffer = std::move(vertexBuffer.buffer);
        mVertexBuffer.memory = std::move(vertexBuffer.memory);
        mIndexBuffer.buffer  = std::move(indexBuffer.buffer);
        mIndexBuffer.memory  = std::move(indexBuffer.memory);

        // create uniform buffer
        mUniformBuffer = utils::createBuffer(mDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(ShaderData),
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vk.MapMemory(dev, *mUniformBuffer.memory, 0, sizeof(ShaderData), 0, &mUniformMapped);

        // cretae ds and pipeline layouts
        mDsLayout       = createDsLayout(mDevice);
        mPipelineLayout = createPipeLayout(mDevice, *mDsLayout);

        // create descriptor
        mDesriptorPool = createDescriptorPool(mDevice);
        mDescriptorSet = createDescriptorSets(mDevice, *mDesriptorPool, *mDsLayout);
        updateDescriptorSets(mDevice, *mDescriptorSet, *mUniformBuffer.buffer);

        // create pipeline
        createPipeline(mDevice, *mPipelineLayout, *mRenderPass);

        printf("");
    }
    //virtual void draw() override {
    //    preDraw();
    //    postDraw();
    //}
};

int main() {
    std::string title{ constants::kWindowTitle };
    title += ": simple triangle";

    tools::IRenderer::Ptr sample = std::make_shared<SimpleTriangle>();
    tools::PolypWindow win{ title.c_str(), 0, 0, 1024, 600, sample };

    win.run();
}