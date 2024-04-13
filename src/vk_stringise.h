#include "vulkan/vulkan_raii.hpp"

#include <string>

namespace polyp {
namespace vulkan {

std::string to_string(const vk::raii::PhysicalDevice& gpu);

}
}
