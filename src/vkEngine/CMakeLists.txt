cmake_minimum_required(VERSION 3.6)

set(sources 
            ${CMAKE_CURRENT_SOURCE_DIR}/swapchain.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/device.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/instance.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/surface.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/example.cpp)

set(includes ${CMAKE_CURRENT_SOURCE_DIR}
             ${VULKANSDK_INCLUDE_DIR})

set(headers 
            swapchain.h
            device.h
            instance.h
            surface.h
            destroyer.h
            common.h
            utils.h
            concepts.h
            dispatch_table.h
            create_infos.h
            example.h)

set(sources ${sources}
            ${headers})

add_library(vkEngine STATIC ${sources})

target_include_directories(vkEngine PUBLIC ${includes})

target_link_libraries(vkEngine tools
                               glm)

set_target_properties(vkEngine PROPERTIES FOLDER "src")
