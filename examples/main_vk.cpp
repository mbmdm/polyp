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
    for (size_t i = 0; i < instance->gpuCount(); i++) {
        auto gpu = instance->gpuInfo(i);
        if (gpu.memory() > physGpu.memory() && gpu.isDiscrete()) {
            physGpu = gpu;
        }
    }
    POLYPINFO("Selected device %s with local memory %d mb\n", physGpu.name().c_str(), physGpu.memory() / 1024 / 1024);

    std::shared_ptr<IRenderer> sample = std::make_shared<Sample>();
    WindowSurface win{ "Anissimus hello vulkan", 0, 0, 1024, 600, sample };
     
    SurfaceCreateInfo info = std::apply([](auto hwnd, auto inst) { 
        return SurfaceCreateInfo{ inst, hwnd };
        }, win.params());
    Surface::Ptr surface = Surface::create(instance, info);
    if (!surface) {
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

    VkPresentModeKHR presentMode = utils::getMostSuitablePresentationMode(instance, surface, physGpu);
    if (presentMode == VK_PRESENT_MODE_MAX_ENUM_KHR) {
        POLYPFATAL("Failed to select desired presentation mode");
    }

    auto surfaceCapabilities = surface->capabilities(physGpu);
    if (surfaceCapabilities.currentExtent.width == UINT32_MAX ||
        surfaceCapabilities.currentExtent.height == UINT32_MAX) {
        POLYPFATAL("Unsupported swapchain creation scenario.");
    }

    auto numberOfImages = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount) {
        numberOfImages = std::min(numberOfImages, surfaceCapabilities.maxImageCount);
    }

    auto swchainUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (surfaceCapabilities.supportedUsageFlags & swchainUsage != swchainUsage) {
        POLYPFATAL("Failed to create swapchain with required usage flags");
    }

    if (surfaceCapabilities.currentTransform & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR == 0) {
        POLYPFATAL("Surface doen't support the required swapchain image transformation");
    }

    auto surfaceFormats = surface->formats(physGpu);
    VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    if (!surface->checkSupport(physGpu, surfaceFormat)) {
        surface->checkSupport(physGpu, surfaceFormat, surfaceFormat);
    }

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> swapchain_images;
    {
        VkSwapchainCreateInfoKHR swapchain_create_info = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, // VkStructureType                 sType
        nullptr,                                     // const void                    * pNext
        0,                                           // VkSwapchainCreateFlagsKHR       flags
        **surface,                                   // VkSurfaceKHR                    surface
        numberOfImages,                              // uint32_t                        minImageCount
        surfaceFormat.format,                        // VkFormat                        imageFormat
        surfaceFormat.colorSpace,                    // VkColorSpaceKHR                 imageColorSpace
        surfaceCapabilities.currentExtent,           // VkExtent2D                      imageExtent
        1,                                           // uint32_t                        imageArrayLayers
        swchainUsage,                                // VkImageUsageFlags               imageUsage
        VK_SHARING_MODE_EXCLUSIVE,                   // VkSharingMode                   imageSharingMode
        0,                                           // uint32_t                        queueFamilyIndexCount
        nullptr,                                     // const uint32_t                * pQueueFamilyIndices
        surfaceCapabilities.currentTransform,        // VkSurfaceTransformFlagBitsKHR   preTransform
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,           // VkCompositeAlphaFlagBitsKHR     compositeAlpha
        presentMode,                                 // VkPresentModeKHR                presentMode
        VK_TRUE,                                     // VkBool32                        clipped
        swapchain                                   // VkSwapchainKHR                  oldSwapchain
        };
        CHECKRET(device->dispatchTable().CreateSwapchainKHR(**device, &swapchain_create_info, nullptr, &swapchain));
        if (swapchain == VK_NULL_HANDLE) {
            POLYPFATAL("Failed to create swapchain.");
        }

        uint32_t images_count = 0;
        VkResult result = VK_SUCCESS;
        CHECKRET(device->dispatchTable().GetSwapchainImagesKHR(**device, swapchain, &images_count, nullptr));
        swapchain_images.resize(images_count);
        CHECKRET(device->dispatchTable().GetSwapchainImagesKHR(**device, swapchain, &images_count, swapchain_images.data()));
    }

    win.run();
}
