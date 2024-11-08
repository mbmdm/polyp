#pragma once

#include "vk_context.h"
#include "application.h"

#define RUN_APP_EXAMPLE(ClassName)                                                                  \
std::string title{ constants::kWindowTitle };                                                       \
title += ": "#ClassName;                                                                            \
                                                                                                    \
ClassName sample{};                                                                                 \
                                                                                                    \
Application::get().onWindowInitialized += [&sample](const auto& args) { sample.onInit(args); };     \
Application::get().onWindowResized     += [&sample](const auto& args) { sample.onResize(args); };   \
Application::get().onKeyPress          += [&sample](const auto& args) { sample.onKeyPress(args); }; \
Application::get().onShutdown          += [&sample]() { sample.onShoutDown(); };                    \
Application::get().onRender            += [&sample]() { sample.onRender(); };                       \
                                                                                                    \
Application::get().init(title.c_str(), 1024, 600);                                                  \
Application::get().run();

namespace polyp {
namespace vulkan {
namespace example {

class ExampleBase
{
public:
    virtual ~ExampleBase() = default;

    void onRender();
    bool onInit(const WindowInitializedEventArgs& args);
    bool onResize(const WindowResizeEventArgs& args);
    void onKeyPress(const KeyPressEventArgs& args);
    void onShoutDown();

protected:
    struct MVP
    {
        glm::mat4 projectionMatrix;
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
    };

    struct SubmitInfo
    {
        vk::Fence fence = VK_NULL_HANDLE;
        std::vector<vk::CommandBuffer> cmds;
    };

    MVP getMVP() const;

    virtual bool                           postInit() = 0;
    virtual bool                         postResize() = 0;
    virtual SubmitInfo                 getSubmitCmd() = 0;
    virtual RHIContext::CreateInfo getRHICreateInfo() = 0;

    Queue                  mQueue           = { VK_NULL_HANDLE };
    CommandPool            mCmdPool         = { VK_NULL_HANDLE };
    uint32_t               mCurrSwImIndex   = {};
    std::vector<vk::Image> mSwapChainImages = {};
    std::vector<ImageView> mSwapChainVeiews = {};

private:
    void acquireNextSwapChainImage();
    void submit(const std::vector<vk::CommandBuffer>& cmds, vk::Fence fence = VK_NULL_HANDLE);
    void present();

    Fence                  mAqImageFence = { VK_NULL_HANDLE };
    std::vector<Semaphore> mSemaphores   = {};
    RHIContext::CreateInfo mContextInfo  = {};
    bool                   mPauseDrawing = false;

    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 3.0f);   // TODO: pack into camer class
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);  // TODO: pack into camer class
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);   // TODO: pack into camer class
};

} // example
} // vulkan
} // polyp
