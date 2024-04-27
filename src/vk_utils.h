//#include <vulkan/vulkan_raii.hpp>
#include "vk_common.h"

#include <vector>

namespace polyp {
namespace vulkan {
namespace utils {

//bool isDiscrete(const PhysicalDevice& gpu);

//VkDeviceSize getMemorySize(const PhysicalDevice& gpu);

PhysicalDevice getPowerfullGPU(const std::vector<PhysicalDevice>& gpus);

//bool checkSupport(const vk::raii::PhysicalDevice& gpu, vk::Format format);

bool checkSupport(const PhysicalDevice& device, const SurfaceKHR& surface, PresentModeKHR mode);

}
}
}
