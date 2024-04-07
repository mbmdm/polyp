cmake_minimum_required(VERSION 3.6)

add_subdirectory(tools)

set(sources
            #${CMAKE_CURRENT_SOURCE_DIR}/vk/destroyable_handle.cpp
            #${CMAKE_CURRENT_SOURCE_DIR}/vk/swapchain.cpp
            #${CMAKE_CURRENT_SOURCE_DIR}/vk/device.cpp
            #${CMAKE_CURRENT_SOURCE_DIR}/vk/instance.cpp
            #${CMAKE_CURRENT_SOURCE_DIR}/vk/surface.cpp
            #${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
            #${CMAKE_CURRENT_SOURCE_DIR}/example.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/example_raii.cpp)

set(includes ${CMAKE_CURRENT_SOURCE_DIR}/
             ${CMAKE_CURRENT_SOURCE_DIR}
             ${CMAKE_CURRENT_SOURCE_DIR}/vk
             ${VULKANSDK_INCLUDE_DIR})

set(headers
            #${CMAKE_CURRENT_SOURCE_DIR}/vk/swapchain.h
            #${CMAKE_CURRENT_SOURCE_DIR}/vk/device.h
            #${CMAKE_CURRENT_SOURCE_DIR}/vk/instance.h
            #${CMAKE_CURRENT_SOURCE_DIR}/vk/surface.h
            #${CMAKE_CURRENT_SOURCE_DIR}/vk/dispatch_table.h
            #${CMAKE_CURRENT_SOURCE_DIR}/vk/destroyable_handle.h
            ${CMAKE_CURRENT_SOURCE_DIR}/common.h
            #destroyer.h
            #utils.h
            #concepts.h
            #create_infos.h
            #example.h
             ${CMAKE_CURRENT_SOURCE_DIR}/example_raii.h)

set(sources ${sources}
            ${headers})

add_library(vkEngine STATIC ${sources})

target_include_directories(vkEngine PUBLIC ${includes})

target_link_libraries(vkEngine tools
                               glm)

set_target_properties(vkEngine PROPERTIES FOLDER "src")
