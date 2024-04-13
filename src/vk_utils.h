#include "vulkan/vulkan_raii.hpp"

#include <vector>

namespace polyp {
namespace vulkan {

bool isDiscrete(const vk::raii::PhysicalDevice& gpu);

VkDeviceSize getMemorySize(const vk::raii::PhysicalDevice& gpu);

vk::raii::PhysicalDevice getPowerfullGPU(const std::vector<vk::raii::PhysicalDevice>& gpus);

std::vector<bool> getSupportedQueueFamilies(const vk::raii::PhysicalDevice& gpu, vk::QueueFlags flags, uint32_t count);

std::vector<bool> getSupportedQueueFamilies(const vk::raii::PhysicalDevice& gpu, const vk::raii::SurfaceKHR& surface);

}
}
