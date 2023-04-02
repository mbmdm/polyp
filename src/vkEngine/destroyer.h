#ifndef VK_DESTROYER_H
#define VK_DESTROYER_H

#include "common.h"
#include "concepts.h"

namespace polyp {
namespace engine {

// Wrappers block

// Macro for desctoying objects which descroy func takes the object handle as first argument
#define DECLARE_VKDESCTOYER_WRAPPED( VkType, HandleType )                \
struct VkType##Wrapper { HandleType mHandle; };

/// Macro for desctoying objects which descroy func takes the vk object root handle as first argument
/// For instance, vkDestroySurfaceKHR takes VkDevice and VkSurfaceKHR to descroy the second one
#define DECLARE_VKDESCTOYER_CHILD_WRAPPED( VkTypeRoot, VkTypeChild)      \
struct VkTypeChild##Wrapper { VkTypeRoot mRoot; VkTypeChild mHandle; };

DECLARE_VKDESCTOYER_WRAPPED(VkInstance, VkInstance);
DECLARE_VKDESCTOYER_WRAPPED(VkLibrary,  HMODULE);
DECLARE_VKDESCTOYER_WRAPPED(VkDevice,   VkDevice);
DECLARE_VKDESCTOYER_CHILD_WRAPPED(VkInstance, VkSurfaceKHR);
DECLARE_VKDESCTOYER_CHILD_WRAPPED(VkDevice,   VkSwapchainKHR);
DECLARE_VKDESCTOYER_CHILD_WRAPPED(VkDevice,   VkCommandPool);
DECLARE_VKDESCTOYER_CHILD_WRAPPED(VkDevice,   VkFence);
DECLARE_VKDESCTOYER_CHILD_WRAPPED(VkDevice,   VkSemaphore);

// Destroy funcs

template<typename PFN_Destroy, typename VkTypeWrapper, typename... Args>
inline void destroyVulkanObject(PFN_Destroy pFun, VkTypeWrapper object, Args... args) {
    if constexpr (has_field_mRoot<VkTypeWrapper>) {
        pFun(object.mRoot, object.mHandle, args...);
    }
    else {
        pFun(object.mHandle, args...);
    }
}

// VkDestroyer class

template<class VkTypeWrapper> //TODO: try to use concept
class VkDestroyer {
public:
    VkDestroyer() : mDestroyFunc(nullptr) {
        mObject.mHandle = VK_NULL_HANDLE;
    }

    VkDestroyer(std::function<void(VkTypeWrapper)> deleter) : mDestroyFunc(deleter) {
        mObject.mHandle = VK_NULL_HANDLE;
    }

    VkDestroyer(VkTypeWrapper object, std::function<void(VkTypeWrapper)> deleter) : mDestroyFunc(deleter) {
        mObject.mHandle = object.mHandle;
        if constexpr (has_field_mRoot<VkTypeWrapper>) {
            mObject.mRoot = object.mRoot;
        }
    }

    ~VkDestroyer() {
        if (mDestroyFunc && mObject.mHandle) {
            auto handleVal = reinterpret_cast<uint64_t*>(mObject.mHandle);
            POLYPDEBUG("Desctoying object %s %lu", typeid(decltype(mObject.mHandle)).name(), mObject.mHandle);
            mDestroyFunc(mObject);
        }
    }

    VkDestroyer(VkDestroyer<VkTypeWrapper>&& other) : mDestroyFunc(other.mDestroyFunc) {
        mObject.mHandle = other.mObject.mHandle;
        other.mObject.mHandle = VK_NULL_HANDLE;
        other.mDestroyFunc = nullptr;
        if constexpr (has_field_mRoot<VkTypeWrapper>) {
            mObject.mRoot = other.mObject.mRoot;
            other.mObject.mRoot = VK_NULL_HANDLE;
        }
    }

    VkDestroyer& operator=(VkDestroyer<VkTypeWrapper>&& other) {
        
        if (this == &other) {
            return *this;
        }

        VkTypeWrapper object = mObject;
        std::function<void(VkTypeWrapper)> destroyFunc = mDestroyFunc;

        mObject.mHandle = other.mObject.mHandle;
        if constexpr (has_field_mRoot<VkTypeWrapper>) {
            mObject.mRoot = other.mObject.mRoot;
        }
        mDestroyFunc = other.mDestroyFunc;

        other.mObject.mHandle = object.mHandle;
        other.mDestroyFunc = destroyFunc;
        if constexpr (has_field_mRoot<VkTypeWrapper>) {
            other.mObject.mRoot = object.mRoot;
        }

        return *this;
    }

    decltype(VkTypeWrapper::mHandle)& operator*() {
        return mObject.mHandle;
    }

    decltype(VkTypeWrapper::mHandle) const& operator*() const {
        return mObject.mHandle;
    }

    bool operator!() const {
        return mObject.mHandle == VK_NULL_HANDLE;
    }

    operator bool() const {
        return mObject.mHandle != VK_NULL_HANDLE;
    }

    VkDestroyer(VkDestroyer<VkTypeWrapper> const&) = delete;
    VkDestroyer& operator=(VkDestroyer<VkTypeWrapper> const&) = delete;

private:
    VkTypeWrapper mObject;
    std::function<void(VkTypeWrapper)> mDestroyFunc;
};

// VkDestroyer macro to simplify class member declaration

#define DECLARE_VKDESTROYER( VkType ) VkDestroyer<VkType##Wrapper>

// Init vk destroyers

template<typename PFN_VK_TYPE, typename VkTypeWrapper, typename... Args>
inline void initVkDestroyer(PFN_VK_TYPE pfnDestroy, VkDestroyer<VkTypeWrapper>& wrapped, Args... args) {

    using namespace std::placeholders;
    VkTypeWrapper object;
    memcpy(&object, &wrapped, sizeof(VkTypeWrapper)); // backup underlying VkTypeWrapper object. We're gonna recreate VkDestroyer
    auto destroyFunc = std::bind(destroyVulkanObject<PFN_VK_TYPE, VkTypeWrapper, Args...>, pfnDestroy, _1, args...);
    wrapped = VkDestroyer<VkTypeWrapper>(object, destroyFunc);
}

} // engine
} // polyp

#endif // VK_DESTROYER_H
