cmake_minimum_required(VERSION 3.20)

set(sources
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_common.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_utils.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_context.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/example/example_base.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/example/example_a.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/generic/camera.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/generic/application.cpp)

set(includes
             ${CMAKE_CURRENT_SOURCE_DIR}
             ${CMAKE_CURRENT_SOURCE_DIR}/vk
             ${CMAKE_CURRENT_SOURCE_DIR}/generic
             ${CMAKE_CURRENT_SOURCE_DIR}/example
             ${VULKANSDK_INCLUDE_DIR})

set(headers
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_common.h
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_utils.h
            ${CMAKE_CURRENT_SOURCE_DIR}/vk_context.h
            ${CMAKE_CURRENT_SOURCE_DIR}/common.h
            ${CMAKE_CURRENT_SOURCE_DIR}/generic/fps_counter.h
            ${CMAKE_CURRENT_SOURCE_DIR}/generic/camera.h)

set(sources ${sources}
            ${headers})

add_library(vkEngine STATIC ${sources})

target_include_directories(vkEngine PUBLIC ${includes})

target_link_libraries(vkEngine glm
                               VulkanMemoryAllocator)

set_target_properties(vkEngine PROPERTIES FOLDER "src")
