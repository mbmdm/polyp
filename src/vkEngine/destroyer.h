#ifndef VK_DESTROYER_H
#define VK_DESTROYER_H

#include "dispatch_table.h"
#include "concepts.h"

#include <polyp_logs.h>

#include <functional>

namespace polyp {
namespace engine {

#define DECLARE_VK_DESCTOYABLE(VkType)                                                     \
class Vk##VkType##Destroyable                                                              \
{                                                                                          \
public:                                                                                    \
     Vk##VkType##Destroyable() = default;                                                  \
                                                                                           \
    explicit Vk##VkType##Destroyable(Vk##VkType handle) :                                  \
        mHandle{ handle }                                                                  \
    { }                                                                                    \
                                                                                           \
    template<typename VkRootPtr>                                                           \
    Vk##VkType##Destroyable(Vk##VkType handle, VkRootPtr root) :                           \
        mHandle{ handle }                                                                  \
    { initDestroyer<VkRootPtr>(root); }                                                    \
                                                                                           \
    Vk##VkType##Destroyable(Vk##VkType##Destroyable&& other) :                             \
        mHandle{ other.mHandle },                                                          \
        mDestroyer{ other.mDestroyer }                                                     \
    {                                                                                      \
        other.mHandle = VK_NULL_HANDLE;                                                    \
        other.mDestroyer = nullptr;                                                        \
    }                                                                                      \
                                                                                           \
    ~Vk##VkType##Destroyable()                                                             \
    {                                                                                      \
        if (!mHandle) return;                                                              \
        if (!mDestroyer) {                                                                 \
            POLYPERROR("Descroying of %s failed", "Vk"#VkType"Descroyable");               \
            return;                                                                        \
        }                                                                                  \
        mDestroyer(mHandle);                                                               \
    }                                                                                      \
                                                                                           \
    Vk##VkType##Destroyable(Vk##VkType##Destroyable const&) = delete;                      \
                                                                                           \
    Vk##VkType##Destroyable& operator=(Vk##VkType##Destroyable const&) = delete;           \
                                                                                           \
    Vk##VkType##Destroyable& operator=(Vk##VkType##Destroyable&& other) noexcept           \
    {                                                                                      \
        if (this == &other) return *this;                                                  \
        auto handle = mHandle;                                                             \
        auto descroyer = mDestroyer;                                                       \
        mHandle = other.mHandle;                                                           \
        mDestroyer = other.mDestroyer;                                                     \
        other.mHandle = handle;                                                            \
        other.mDestroyer = descroyer;                                                      \
        return *this;                                                                      \
    }                                                                                      \
                                                                                           \
    Vk##VkType& operator*() noexcept { return mHandle; }                                   \
                                                                                           \
    const Vk##VkType& operator*() const noexcept { return mHandle; }                       \
                                                                                           \
    bool operator!() const noexcept { return mHandle == VK_NULL_HANDLE; }                  \
                                                                                           \
    operator bool() const noexcept { return mHandle != VK_NULL_HANDLE; }                   \
                                                                                           \
    Vk##VkType& native() noexcept { return mHandle; }                                      \
                                                                                           \
    const Vk##VkType& native() const noexcept { return mHandle; }                          \
                                                                                           \
    Vk##VkType* pNative() noexcept { return &mHandle; }                                    \
                                                                                           \
    const Vk##VkType* pNative() const noexcept { return &mHandle; }                        \
                                                                                           \
    void setHandle(Vk##VkType&& handle) { mHandle = std::move(handle); }                   \
                                                                                           \
    void setDestroyer(std::function<void(Vk##VkType)> func) { mDestroyer = func; }         \
                                                                                           \
    template<typename VkRootPtr>                                                           \
    void initDestroyer(VkRootPtr root) {                                                   \
        using namespace std::placeholders;                                                 \
        if constexpr (std::is_same_v<decltype(mHandle), VkInstance> == true ||             \
                      std::is_same_v<decltype(mHandle), VkDevice> == true)                 \
            mDestroyer =                                                                   \
                std::bind(root->vk().Destroy##VkType, _1, nullptr);                        \
        else if constexpr (std::is_same_v<decltype(mHandle), VkDeviceMemory> == true)      \
            mDestroyer =                                                                   \
                std::bind(root->vk().FreeMemory, root->native(), _1, nullptr);             \
        else if constexpr (std::is_same_v<decltype(mHandle), VkCommandBuffer> == true ||   \
                           std::is_same_v<decltype(mHandle), VkQueue> == true         ||   \
                           std::is_same_v<decltype(mHandle), VkDescriptorSet> == true)     \
            notDestroyable();                                                              \
        else                                                                               \
            mDestroyer =                                                                   \
                std::bind(root->vk().Destroy##VkType, root->native(), _1, nullptr);        \
    }                                                                                      \
                                                                                           \
    void notDestroyable() noexcept {                                                       \
        auto pStub = &Vk##VkType##Destroyable::destroyerStub;                              \
        mDestroyer = std::bind(pStub, this, std::placeholders::_1);                        \
    }                                                                                      \
                                                                                           \
private:                                                                                   \
    Vk##VkType mHandle                         = { VK_NULL_HANDLE };                       \
    std::function<void(Vk##VkType)> mDestroyer = { nullptr };                              \
                                                                                           \
    void destroyerStub(Vk##VkType) noexcept {}                                             \
};

template<typename T>
class PlainDestroyable
{
public:
    PlainDestroyable() :
        mHandle{ NULL },
        mDestroyer{ nullptr }
    { }

    PlainDestroyable(T handle):
        mHandle{ handle },
        mDestroyer{ nullptr }
    { }

    ~PlainDestroyable()
    {
        if (!mHandle)
            return;
        if (!mDestroyer) {
            POLYPERROR("Descroying of PlainDestroyable<%s> failed", typeid(T).name());
            return;
        }
        mDestroyer(mHandle);
    }

    PlainDestroyable(const PlainDestroyable&)  = delete;
    PlainDestroyable(PlainDestroyable&&)       = delete;
    T& operator=(const PlainDestroyable&)      = delete;
    T& operator=(PlainDestroyable&&)           = delete;

    bool operator!() const noexcept { return mHandle == NULL; }

    operator bool() const noexcept { return mHandle != NULL; }

    T& operator*() noexcept { return mHandle; }

    const T& operator*() const noexcept { return mHandle; }

    T& native() noexcept { return mHandle; }

    const T& native() const noexcept { return mHandle; }

    T* pNative() noexcept { return &mHandle; }

    const T* pNative() const noexcept { return &mHandle; }

    void setHandle(T&& handle) { mHandle = std::forward(handle); }

    void setDestroyer(std::function<void(T)> func) { mDestroyer = func; }

private:
    T mHandle;
    std::function<void(T)> mDestroyer;
};

using HMODULEDestroyable = PlainDestroyable<HMODULE>;

DECLARE_VK_DESCTOYABLE(Instance);
DECLARE_VK_DESCTOYABLE(Device);
DECLARE_VK_DESCTOYABLE(SurfaceKHR);
DECLARE_VK_DESCTOYABLE(SwapchainKHR);
DECLARE_VK_DESCTOYABLE(CommandPool);
DECLARE_VK_DESCTOYABLE(Fence);
DECLARE_VK_DESCTOYABLE(Semaphore);
DECLARE_VK_DESCTOYABLE(Buffer);
DECLARE_VK_DESCTOYABLE(BufferView);
DECLARE_VK_DESCTOYABLE(DeviceMemory);
DECLARE_VK_DESCTOYABLE(DescriptorSet);
DECLARE_VK_DESCTOYABLE(DescriptorSetLayout);
DECLARE_VK_DESCTOYABLE(Pipeline);
DECLARE_VK_DESCTOYABLE(PipelineLayout);
DECLARE_VK_DESCTOYABLE(Queue);
DECLARE_VK_DESCTOYABLE(CommandBuffer);
DECLARE_VK_DESCTOYABLE(Image);
DECLARE_VK_DESCTOYABLE(ImageView);
DECLARE_VK_DESCTOYABLE(RenderPass);
DECLARE_VK_DESCTOYABLE(Framebuffer);
DECLARE_VK_DESCTOYABLE(DescriptorPool);
DECLARE_VK_DESCTOYABLE(ShaderModule);

#define DESTROYABLE(Type) Type##Destroyable

} // engine
} // polyp

#endif // VK_DESTROYER_H
