#ifndef SURFACE_H
#define SURFACE_H

#include "common.h"
#include "destroyer.h"
#include "instance.h"

#include <polyp_window.h>

namespace polyp {
namespace engine {

struct SurfaceCreateInfo {
    polyp::tools::WindowsInstance mWindowsInstance;
    polyp::tools::WindowsHandle   mWindowsHandle;
};

class Surface final {
private:
    Surface(Instance::Ptr instance, const SurfaceCreateInfo& info);

public:
    using Ptr      = std::shared_ptr<Surface>;
    using ConstPtr = std::shared_ptr<Surface const>;

    Surface(const Surface&)            = delete;
    Surface& operator=(const Surface&) = delete;
    Surface(Surface&&)                 = delete;
    Surface& operator=(Surface&&)      = delete;
    ~Surface()                         = default;

    /// Loops over all gpu queue family indexes and returns true if Surface 
    /// is supported by an apropriate queue family by given index
    std::vector<bool> checkSupport(PhysicalGpu gpu)                  const;
    /// Loops over all supported formats and returns true if supported
    bool checkSupport(PhysicalGpu gpu, VkSurfaceFormatKHR format)    const;
    /// Loops over all supported formats and returns true if supported and
    /// fill out_format with the most suitable in case required is not supported
    bool checkSupport(PhysicalGpu gpu, VkSurfaceFormatKHR in_format,
                      VkSurfaceFormatKHR& out_format)                const;
    /// Loops over all supported presentation modes and returns true if required mode was found
    bool checkSupport(PhysicalGpu gpu, VkPresentModeKHR mode)        const;
    /// Loops over all supported presentation modes and returns true if required mode was found
    /// and fill out_mode with the most suitable in case required is not supported
    bool checkSupport(PhysicalGpu gpu, VkPresentModeKHR in_mode,
                      VkPresentModeKHR out_mode)                     const;
    /// Returns Surface capabilities for SwapChain 
    VkSurfaceCapabilitiesKHR capabilities(PhysicalGpu gpu)           const;
    /// Returns supported surface 
    std::vector<VkSurfaceFormatKHR> format(PhysicalGpu gpu)          const;
    /// Returns supported surface presentation 
    std::vector<VkPresentModeKHR> presentModes(PhysicalGpu gpu)      const;

    /// Creates presenration surface
    /// 
    /// Typical usage:
    /// \code
    ///   create(Instance::Ptr instance, const SurFaceCreateInfo& info);
    /// \endcode
    /// 
    /// \param instance - polyp::engine::Instance::Ptr;
    /// \param info     - additional surface creation info;
    template<typename ...Args>
    [[nodiscard]] static Ptr create(Args... args) {
        std::shared_ptr<Surface> output(new Surface(args...));
        if (!output->init()) {
            output.reset();
        }
        return output;
    }

    /// Returns underlying vulkan handle.
    const VkSurfaceKHR& operator*() const;
    /// Returns underlying vulkan handle.
    VkSurfaceKHR native()           const;

private:
    bool init();

    Instance::Ptr             mInstance;
    SurfaceCreateInfo         mInfo;
    DESTROYABLE(VkSurfaceKHR) mHandle;
};

} // engine
} // polyp

#endif // SURFACE_H
