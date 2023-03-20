#include <camera.h>
#include <error_codes.h>
#include <device.h>
#include <instance.h>
#include <surface.h>
#include <utils.h>
#include <window_surface.h>
#include <polyp_logs.h>

class Sample : public polyp::tools::IRenderer {
public:
    virtual ~Sample() override {
        POLYPINFO(__FUNCTION__);
    }

    virtual bool onInit() override {
        POLYPINFO(__FUNCTION__);
        return true;
    }

    virtual bool onResize() override {
        POLYPINFO(__FUNCTION__);
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

    using namespace polyp::engine;
    using namespace polyp::tools;

    InstanceCreateInfo instanceInfo;
    instanceInfo.mVersion.major = 1;
    Instance::Ptr instance = Instance::create("AnisVkApplication", instanceInfo);
    if (!instance) {
        POLYPFATAL("Failed to create vulkan instance");
    }

    polyp::engine::GpuInfo physGpu;
    for (size_t i = 0; i < instance->getSystemGpuCount(); i++) {
        auto gpu = instance->getSystemGpuInfo(i);
        if (gpu.memory() > physGpu.memory() && gpu.isDiscrete()) {
            physGpu = gpu;
        }
    }
    POLYPINFO("Selected device %s with local memory %d mb\n", physGpu.name().c_str(), physGpu.memory() / 1024 / 1024);

    std::shared_ptr<IRenderer> sample = std::make_shared<Sample>();
    WindowSurface win{ "Anissimus hello vulkan", 0, 0, 800, 800, sample };
     
    SurfaceCreateInfo info = std::apply([](auto hwnd, auto inst) { 
        return SurfaceCreateInfo{ inst, hwnd };
        }, win.getWindowHandle());
    Surface::Ptr surface = Surface::create(instance, info);
    if (!surface /*&& surface->checkSupport(physGpu)*/) {
        POLYPFATAL("Failed to create vulkan surface (WSI)");
    }

    QueueCreateInfo queInfo;
    queInfo.mFamilyIndex = UINT32_MAX;
    queInfo.mQueueType = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
    queInfo.mPriorities = { 1., 0.5 };

    auto availableGraphicsQueue = physGpu.checkSupport(queInfo.mQueueType, queInfo.mPriorities.size());
    auto availableWSIQueue      = surface->checkSupport(physGpu);

    for (uint32_t i = 0; i < physGpu.queueFamilyCount(); i++) {
        if (availableGraphicsQueue[i] && availableWSIQueue[i]) {
            queInfo.mFamilyIndex = i;
            break;
        }
    }
    if (queInfo.mFamilyIndex == ~0) {
        POLYPFATAL("Failed to find the reqired queue family");
    }

    DeviceCreateInfo deviceInfo;
    queInfo.mFamilyIndex = UINT32_MAX;
    deviceInfo.mQueueInfo = { queInfo };
    Device::Ptr device = Device::create(instance, physGpu, deviceInfo);
    if (!device) {
        POLYPFATAL("Failed to create vulkan rendering device");
    }

    win.run();
}
