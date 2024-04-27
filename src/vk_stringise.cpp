#include "vk_stringise.h"
#include "vk_utils.h"

#include <sstream>

using namespace polyp::vulkan::utils;

namespace polyp {
namespace vulkan {

std::string to_string(const PhysicalDevice& gpu)
{
    vk::PhysicalDeviceProperties props = gpu.getProperties();

    std::stringstream ss; 
    ss << props.deviceName;

    auto memory = gpu.getDeviceMemoryPLP();
    ss << ", memory " << memory / 1024 / 1024 << " Mb";

    return ss.str();
}

}
}
