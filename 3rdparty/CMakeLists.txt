cmake_minimum_required(VERSION 3.20)

#Building GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
set_target_properties(glfw PROPERTIES FOLDER "3rdParty/glfw")
set_target_properties(uninstall PROPERTIES FOLDER "3rdParty/glfw")
set_target_properties(update_mappings PROPERTIES FOLDER "3rdParty/glfw")

#Building GLAD
find_package(Python3 COMPONENTS Interpreter)
execute_process(COMMAND ${Python3_EXECUTABLE} -c "import jinja2" RESULT_VARIABLE res_var)
if(res_var EQUAL 0)
	message(STATUS "Python3 check passed.")
else()
	message(FATAL_ERROR "Python10 with module jinja2 is required!")
endif()
add_subdirectory(glad/cmake)
set(GLAD_BUILD_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/gladsources)
glad_add_library(glad_gl_core_46 STATIC LOCATION ${GLAD_BUILD_LOCATION} LANGUAGE C API gl:core=4.6)
set_target_properties(glad_gl_core_46 PROPERTIES FOLDER "3rdParty/glad")

#GLM
add_subdirectory(glm)

#STB
set(STB_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/stb CACHE PATH "Path to STB headers")

#Vulkan SDK
find_package(VulkanSDK REQUIRED FATAL_ERROR)

#Vulkan-Headers
set(VULKAN_HEADERS_INCLUDE_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/vulkan_headers/include CACHE PATH "Path to Vulkan-Headers include")

#Vulkan Memory Allocator (VMA)
add_subdirectory(vma)