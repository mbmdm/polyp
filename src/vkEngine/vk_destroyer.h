#ifndef VK_DESTROYER_H
#define VK_DESTROYER_H

#include "common.h"

namespace polyp {
namespace engine {

// Wrappers block

#define DECLARE_VKDESCTOYER_WRAPPED( VkType, HandleType ) \
struct VkType##Wrapper { HandleType mHandle; };

DECLARE_VKDESCTOYER_WRAPPED(VkInstance, VkInstance);
DECLARE_VKDESCTOYER_WRAPPED(VkLibrary, HMODULE);
DECLARE_VKDESCTOYER_WRAPPED(VkDevice, VkDevice);

// Destroy funcs

template<typename PFN_Destroy, typename VkTypeWrapper, typename... Args>
inline void destroyVulkanObject(PFN_Destroy pFun, VkTypeWrapper object, Args... args) {
    pFun(object.mHandle, args...);
}

// VkDestroyer class

template<class VkTypeWrapper> //TODO: try to use concept
class VkDestroyer {
public:
    VkDestroyer() :
        mDestroyFunc(nullptr) {
        mObject.mHandle = VK_NULL_HANDLE;
    }

    VkDestroyer(std::function<void(VkTypeWrapper)> deleter) :
        mDestroyFunc(deleter) {
        mObject.mHandle = VK_NULL_HANDLE;
    }

    VkDestroyer(VkTypeWrapper object, std::function<void(VkTypeWrapper)> deleter) :
        mDestroyFunc(deleter) {
        mObject.mHandle = object.mHandle;
    }

    ~VkDestroyer() {
        if (mDestroyFunc && mObject.mHandle) {
            //TODO: need special macro for this perpos in common.h (like RDCDEBUG/RDCINFO...)
            printf("Desctoying object %s %lu\n",
                typeid(decltype(mObject.mHandle)).name(),
                mObject);
            mDestroyFunc(mObject);
        }
    }

    VkDestroyer(VkDestroyer<VkTypeWrapper>&& other) :
        mDestroyFunc(other.mDestroyFunc) {
        mObject.mHandle = other.mObject.mHandle;
        other.mObject.mHandle = VK_NULL_HANDLE;
        other.mDestroyFunc = nullptr;
    }

    VkDestroyer& operator=(VkDestroyer<VkTypeWrapper>&& other) {
        
        if (this == &other) {
            return *this;
        }

        VkTypeWrapper object = mObject;
        std::function<void(VkTypeWrapper)> destroyFunc = mDestroyFunc;

        mObject.mHandle = other.mObject.mHandle;
        mDestroyFunc = other.mDestroyFunc;

        other.mObject.mHandle = object.mHandle;
        other.mDestroyFunc = destroyFunc;

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
    memcpy(&object, &wrapped, sizeof(VkTypeWrapper));
    auto destroyFunc = std::bind(destroyVulkanObject<PFN_VK_TYPE, VkTypeWrapper, Args...>, pfnDestroy, _1, args...);
    wrapped = VkDestroyer<VkTypeWrapper>(object, destroyFunc);
}

} // engine
} // polyp

#endif // VK_DESTROYER_H
