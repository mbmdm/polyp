unset(VULKANSDK_FOUND CACHE) # output
unset(VULKANSDK_INCLUDE_DIR CACHE) # output

STRING(REGEX REPLACE "\\\\" "/" VK_SDK_PATH "$ENV{VK_SDK_PATH}")
find_path( VULKANSDK_INCLUDE_DIR vulkan/vulkan.h ${VK_SDK_PATH}/include )

if(EXISTS "${VK_SDK_PATH}/Include/vulkan/vulkan.h")
  SET(VULKANSDK_INCLUDE_DIR "${VK_SDK_PATH}/Include" CACHE PATH "Path to the vulkan.h header file")
elseif(EXISTS "${VK_SDK_PATH}/include/vulkan/vulkan.h")
  set(VULKANSDK_INCLUDE_DIR "${VK_SDK_PATH}/include" CACHE PATH "Path to the vulkan.h header file")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8 AND EXISTS "${VK_SDK_PATH}/Bin/glslangValidator.exe")
    message(VERBOSE "Using 64-bit glslangValidator") 
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe" CACHE PATH "Path to glslangValidator")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4 AND EXISTS "${VK_SDK_PATH}/Bin32/glslangValidator.exe")
    message(VERBOSE "Using 32-bit glslangValidator")
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe" CACHE PATH "Path to glslangValidator")
else()
    message(ERROR "Failed to find glslangValidator")
endif()

#SET(VULKANSDK_INCLUDE_DIR "${VK_SDK_PATH}/Include" CACHE PATH "Path to the vulkan.h header file")

if (VK_SDK_PATH AND VULKANSDK_INCLUDE_DIR)
    set( VULKANSDK_FOUND "YES" CACHE STRING "Indicates that Vulkan SDK is present/absent" )
    message(STATUS "Found Vulkan SDK    : ${VK_SDK_PATH}")
else()
    set( VULKANSDK_FOUND "NO" CACHE STRING "Indicates that Vulkan SDK is present/absent")
    message(ERROR "
    Failed to find Vulkan SDK. Please download and install ti from the following 
    link https://vulkan.lunarg.com/sdk/home or ensure that the environemnt VK_SDK_PATH is correct.")
endif()

mark_as_advanced( VULKANSDK_FOUND )
