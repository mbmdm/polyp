cmake_minimum_required(VERSION 3.6)

add_subdirectory(tools)

set(sources
            ${CMAKE_CURRENT_SOURCE_DIR}/common/application.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_common.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_stringise.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_utils.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_context.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/example_base.cpp)

set(includes
             ${CMAKE_CURRENT_SOURCE_DIR}
             ${CMAKE_CURRENT_SOURCE_DIR}/vk
             ${CMAKE_CURRENT_SOURCE_DIR}/common
             ${VULKANSDK_INCLUDE_DIR})

set(headers
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_common.h
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_stringise.h
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_utils.h
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_context.h
            ${CMAKE_CURRENT_SOURCE_DIR}/common.h
             ${CMAKE_CURRENT_SOURCE_DIR}/example_base.h)

set(sources ${sources}
            ${headers})

add_library(vkEngine STATIC ${sources})

target_include_directories(vkEngine PUBLIC ${includes})

target_link_libraries(vkEngine tools
                               glm
                               VulkanMemoryAllocator)

set_target_properties(vkEngine PROPERTIES FOLDER "src")
