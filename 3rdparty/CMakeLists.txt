cmake_minimum_required(VERSION 3.20)

#GLM
add_subdirectory(glm)

#STB
set(STB_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/stb CACHE PATH "Path to STB headers")

#Vulkan SDK
find_package(VulkanSDK REQUIRED FATAL_ERROR)

#Vulkan Memory Allocator (VMA)
add_subdirectory(vma)

#Ttinyobjloader
add_subdirectory(tinyobjloader)
set_target_properties(tinyobjloader PROPERTIES FOLDER "3rdpatry")
