#ifndef DISPATCH_TABLE_H
#define DISPATCH_TABLE_H

#include "common_targets.h"

namespace polyp {
namespace engine {

struct DispatchTable {
#define EXPORTED_VULKAN_FUNCTION( name ) PFN_vk##name name = nullptr;
#define GLOBAL_LEVEL_VULKAN_FUNCTION( name ) PFN_vk##name name = nullptr;
#define INSTANCE_LEVEL_VULKAN_FUNCTION( name ) PFN_vk##name name = nullptr;
#define INSTANCE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension) PFN_vk##name name = nullptr;
#define DEVICE_LEVEL_VULKAN_FUNCTION( name ) PFN_vk##name name = nullptr;
#define DEVICE_LEVEL_VULKAN_FUNCTION_FROM_EXTENSION( name, extension) PFN_vk##name name = nullptr;
#include "dispatch_table.inl"
};

} // engine
} // polyp

#endif // DISPATCH_TABLE_H