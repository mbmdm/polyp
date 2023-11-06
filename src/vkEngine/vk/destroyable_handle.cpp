#include "destroyable_handle.h"

#include "device.h"

namespace polyp {
namespace vk {

#define DEFINE_VK_DEVICE_DESTOYER(VkType)                                            \
template<>                                                                           \
void DestroyableHandle<Vk##VkType>::Destroy()                                        \
{                                                                                    \
    auto device = mDevice.lock();                                                    \
    if (!device)                                                                     \
    {                                                                                \
        POLYPERROR("Descroying of %s failed because VkDevice is empty",              \
                   typeid(Vk##VkType).name());                                       \
        return;                                                                      \
    }                                                                                \
    device->vk().Destroy##VkType(**device, mInternal, nullptr);                      \
}                                                                                    \

#define DEFINE_VK_INSTANCE_DESTOYER(VkType)                                          \
template<>                                                                           \
void DestroyableHandle<Vk##VkType>::Destroy()                                        \
{                                                                                    \
    auto instance = mInstance.lock();                                                \
    if (!instance)                                                                   \
    {                                                                                \
        POLYPERROR("Descroying of %s failed because VkDevice is empty",              \
                   typeid(Vk##VkType).name());                                       \
        return;                                                                      \
    }                                                                                \
    instance->vk().Destroy##VkType(**instance, mInternal, nullptr);                  \
}                                                                                    \

#define DEFINE_VK_ROOT_DESTOYER(VkType)                                              \
template<>                                                                           \
void DestroyableHandle<Vk##VkType>::Destroy()                                        \
{                                                                                    \
    if (auto instance = mInstance.lock(); instance)                                  \
        instance->vk().Destroy##VkType(mInternal, nullptr);                          \
    else if (auto device = mDevice.lock(); device)                                   \
        device->vk().Destroy##VkType(mInternal, nullptr);                            \
    else                                                                             \
        POLYPERROR("Descroying of %s failed because VkDevice is empty",              \
                   typeid(Vk##VkType).name());                                       \
}                                                                                    \

#define DEFINE_VK_EMPTY_DESTOYER(VkType)                                             \
template<>                                                                           \
void DestroyableHandle<Vk##VkType>::Destroy() { }                                    \

template<>
void DestroyableHandle<VkDeviceMemory>::Destroy() 
{
    auto device = mDevice.lock();
    if (!device)
    {
        POLYPERROR("Descroying of VkDeviceMemory failed because VkDevice is empty");
        return;
    }
    device->vk().FreeMemory(**device, mInternal, nullptr);
}

DEFINE_VK_ROOT_DESTOYER(Instance);
DEFINE_VK_ROOT_DESTOYER(Device);

DEFINE_VK_INSTANCE_DESTOYER(SurfaceKHR);

DEFINE_VK_DEVICE_DESTOYER(SwapchainKHR);
DEFINE_VK_DEVICE_DESTOYER(CommandPool);
DEFINE_VK_DEVICE_DESTOYER(Fence);
DEFINE_VK_DEVICE_DESTOYER(Semaphore);
DEFINE_VK_DEVICE_DESTOYER(Buffer);
DEFINE_VK_DEVICE_DESTOYER(DescriptorSetLayout);
DEFINE_VK_DEVICE_DESTOYER(Pipeline);
DEFINE_VK_DEVICE_DESTOYER(PipelineLayout);
DEFINE_VK_DEVICE_DESTOYER(Image);
DEFINE_VK_DEVICE_DESTOYER(ImageView);
DEFINE_VK_DEVICE_DESTOYER(RenderPass);
DEFINE_VK_DEVICE_DESTOYER(Framebuffer);
DEFINE_VK_DEVICE_DESTOYER(DescriptorPool);
DEFINE_VK_DEVICE_DESTOYER(ShaderModule);

DEFINE_VK_EMPTY_DESTOYER(DescriptorSet);
DEFINE_VK_EMPTY_DESTOYER(Queue);
DEFINE_VK_EMPTY_DESTOYER(CommandBuffer);

} // engine
} // polyp
