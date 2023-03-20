#ifndef SURFACE_H
#define SURFACE_H

#include "common.h"
#include "destroyer.h"
#include "instance.h"

#include <window_surface.h>

namespace polyp {
namespace engine {

struct SurfaceCreateInfo{
    struct {
        polyp::tools::WindowsInstance inst;
        polyp::tools::WindowsHandle hwnd;
    } mWindow;
};

class Surface final {
private:
    Surface(Instance::Ptr instance, const SurfaceCreateInfo& info);

public:
    using Ptr = std::shared_ptr<Surface>;

    Surface(const Surface&)            = delete;
    Surface& operator=(const Surface&) = delete;
    Surface(Surface&&)                 = delete;
    Surface& operator=(Surface&&)      = delete;
    ~Surface()                         = default;

    /// Loops over all gpu queue family indexes and returns true if supported
    [[nodiscard]] std::vector<bool> checkSupport(GpuInfo gpuInfo)               const;
    /// Loops over all supported formats and returns true is supported
    [[nodiscard]] bool checkSupport(GpuInfo gpuInfo, VkSurfaceFormatKHR format) const;
    /// Loops over all supported formats and returns true is supported and
    /// fill out_format with the most suitable in case required is not supported
    [[nodiscard]] bool checkSupport(GpuInfo gpuInfo, VkSurfaceFormatKHR in_format,
                                    VkSurfaceFormatKHR& out_format)             const;
    /// Returns Surface capabilities for SwapChain creation
    [[nodiscard]] VkSurfaceCapabilitiesKHR capabilities(GpuInfo gpuInfo)        const;
    /// Returns supported surface formats
    [[nodiscard]] std::vector<VkSurfaceFormatKHR> formats(GpuInfo gpuInfo)      const;


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
    static Ptr create(Args... args) {
        std::shared_ptr<Surface> output(new Surface(args...));
        if (!output->init()) {
            output.reset();
        }
        return output;
    }

    const VkSurfaceKHR& operator*() const;

private:
    [[nodiscard]] bool init();

    Instance::Ptr                     mInstance;
    SurfaceCreateInfo                 mInfo;
    DECLARE_VKDESTROYER(VkSurfaceKHR) mHandle;
};

} // engine
} // polyp

#endif // SURFACE_H
