#include <camera.h>
#include <error_codes.h>
#include <instance.h>
#include <surface.h>
#include <device.h>
#include <swapchain.h>
#include <utils.h>
#include <window_surface.h>
#include <polyp_logs.h>

using namespace polyp::engine;
using namespace polyp::tools;

class Sample : public polyp::tools::IRenderer {
private:
    Instance::Ptr  mInstance;
    Device::Ptr    mDevice;
    Swapchain::Ptr mSwapchain;

public:
    virtual ~Sample() override {
        POLYPINFO(__FUNCTION__);
    }

    virtual bool onInit(WindowsInstance inst, WindowsHandle hwnd) override {
        POLYPINFO(__FUNCTION__);

        InstanceCreateInfo instanceInfo;
        instanceInfo.mVersion.major = 1;
        mInstance = Instance::create("AnisVkApplication", instanceInfo);
        if (!mInstance) {
            POLYPFATAL("Failed to create vulkan instance");
            return false;
        }

        polyp::engine::PhysicalGpu physGpu;
        for (size_t i = 0; i < mInstance->gpuCount(); i++) {
            auto gpu = mInstance->gpu(i);
            if (gpu.memory() > physGpu.memory() && gpu.isDiscrete()) {
                physGpu = gpu;
            }
        }
        POLYPINFO("Selected device %s with local memory %d mb\n", physGpu.name().c_str(), physGpu.memory() / 1024 / 1024);

        SurfaceCreateInfo info{ inst, hwnd };
        Surface::Ptr surface = Surface::create(mInstance, info);
        if (!surface) {
            POLYPFATAL("Failed to create vulkan surface (WSI)");
            return false;
        }

        QueueCreateInfo queInfo;
        queInfo.mFamilyIndex = UINT32_MAX;
        queInfo.mQueueType = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
        queInfo.mPriorities = { 1., 0.5 };

        auto availableGraphicsQueue = physGpu.checkSupport(queInfo.mQueueType, queInfo.mPriorities.size());
        auto availableWSIQueue = surface->checkSupport(physGpu);

        for (uint32_t i = 0; i < physGpu.queueFamilyCount(); i++) {
            if (availableGraphicsQueue[i] && availableWSIQueue[i]) {
                queInfo.mFamilyIndex = i;
                break;
            }
        }
        if (queInfo.mFamilyIndex == ~0) {
            POLYPFATAL("Failed to find the reqired queue family");
            return false;
        }

        DeviceCreateInfo deviceInfo;
        queInfo.mFamilyIndex = UINT32_MAX;
        deviceInfo.mQueueInfo = { queInfo };
        mDevice = Device::create(mInstance, physGpu, deviceInfo);
        if (!mDevice) {
            POLYPFATAL("Failed to create vulkan rendering device");
            return false;
        }

        SwapChainCreateInfo swInfo;
        swInfo.presentationMode = VK_PRESENT_MODE_MAILBOX_KHR;
        mSwapchain = Swapchain::create(mDevice, surface, swInfo);
        if (!mSwapchain) {
            POLYPFATAL("Failed to create vulkan swap chain.");
            return false;
        }

        return true;
    }

    virtual bool onResize() override {
        POLYPINFO(__FUNCTION__);
        mSwapchain->update();
        return true;
    }

    virtual void onMouseClick(uint32_t button, bool state) override {
        POLYPINFO(__FUNCTION__);
    }

    virtual void onMouseMove(int x, int y) override {
        //POLYPINFO(__FUNCTION__);
    }

    virtual void onMouseWheel(float value) override {
        POLYPINFO(__FUNCTION__);
    }

    virtual void onShoutDown() override {
        POLYPINFO(__FUNCTION__);
    }

    virtual bool isReady() override {
        //POLYPINFO(__FUNCTION__);
        return true;
    }

    virtual void draw() override {
        //POLYPINFO(__FUNCTION__);
    }
    
    virtual void updateTimer() override {
        //POLYPINFO(__FUNCTION__);
    }

    virtual void mouseReset() override {
        //POLYPINFO(__FUNCTION__);
    }
};

int main() {

    IRenderer::Ptr sample = std::make_shared<Sample>();
    WindowSurface win{ "Anissimus hello vulkan", 0, 0, 1024, 600, sample };
    win.run();
}
