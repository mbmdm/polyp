#pragma once

#include "vk_context.h"
#include "application.h"
#include "fps_counter.h"
#include "camera.h"

#define RUN_APP_EXAMPLE(ClassName)                                                                             \
std::string title{ POLYP_WIN_TITLE };                                                                  \
title += ": "#ClassName;                                                                                       \
                                                                                                               \
try                                                                                                            \
{                                                                                                              \
    ClassName sample{};                                                                                        \
                                                                                                               \
    Application::get().onWindowInitialized += [&sample](const auto& args) { sample.onInit(args); };            \
    Application::get().onWindowResized     += [&sample](const auto& args) { sample.onResize(args); };          \
    Application::get().onMovement          += [&sample](const auto& args) { sample.onMovement(args); };        \
    Application::get().onMouseClick        += [&sample](const auto& args) { sample.onMouseClick(args); };      \
    Application::get().onShutdown          += [&sample]()                 { sample.onShoutDown(); };           \
    Application::get().onRender            += [&sample]()                 { sample.onRender(); };              \
                                                                                                               \
    Application::get().init(title.c_str(), 1024, 600);                                                         \
    Application::get().run();                                                                                  \
}                                                                                                              \
catch (const SystemError& err)                                                                                 \
{                                                                                                              \
    POLYPFATAL("Exception %d (%s), message %s", err.code().value(), err.code().message().c_str(), err.what()); \
    return false;                                                                                              \
}                                                                                                              \
catch (...)                                                                                                    \
{                                                                                                              \
    POLYPFATAL("Internal fatal error.");                                                                       \
    return false;                                                                                              \
}

namespace polyp {
namespace vulkan {
namespace example {

class ExampleBase
{
public:
    ExampleBase() :
       mCamera{ constants::kCameraInitPos, constants::kCameraWorldUp, constants::kCameraInitLookAt }
    {
        mCamera.speed(constants::kMoveSpeed);
        mCamera.sensitivity(constants::kSensitivity);
    }

    virtual ~ExampleBase() = default;

    void onRender();
    void onShoutDown();
    bool onInit(const WindowInitializedEventArgs& args);
    bool onResize(const WindowResizeEventArgs& args);
    void onMouseClick(const MouseClickEventArgs& args);
    void onMovement(const MovementEventArgs& args);

protected:
    struct MVP
    {
        glm::mat4 projectionMatrix;
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
    };

    MVP getMVP();

    virtual void                   draw()             = 0;
    virtual bool                   postInit()         = 0;
    virtual bool                   postResize()       = 0;
    virtual RHIContext::CreateInfo getRHICreateInfo() = 0;

    Queue                      mQueue           = { VK_NULL_HANDLE };
    CommandPool                mCmdPool         = { VK_NULL_HANDLE };
    std::vector<CommandBuffer> mDrawCmds        = {};
    std::vector<Fence>         mDrawFences      = {};
    uint32_t                   mCurrSwImIndex   = {};
    std::vector<vk::Image>     mSwapChainImages = {};
    std::vector<ImageView>     mSwapChainViews  = {};
    FPSCounter                 mFPSCounter;
    Camera                     mCamera;

private:
    void submit();
    void present();
    void waitForFence();
    void createDrawCmds();
    void acquireNextSwapChainImage();

    Fence                  mAqImageFence  = { VK_NULL_HANDLE };
    std::vector<Semaphore> mSemaphores    = {};
    RHIContext::CreateInfo mContextInfo   = {};
    float                  mLastXMousePos = 0.0;
    float                  mLastYMousePos = 0.0;
    bool                   mPauseDrawing  = false;
    bool                   mMouseMoving   = false;
};

} // example
} // vulkan
} // polyp
