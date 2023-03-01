cmake_minimum_required(VERSION 3.6)

set(sources ${CMAKE_CURRENT_SOURCE_DIR}/device.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/instance.cpp)

set(includes ${CMAKE_CURRENT_SOURCE_DIR}
             ${VULKANSDK_INCLUDE_DIR})

set(headers device.h
            instance.h
            vk_destroyer.h
            common.h
            dispatch_table.h)

set(sources ${sources}
            ${headers})

add_library(vkEngine STATIC ${sources})

target_include_directories(vkEngine PUBLIC ${includes})

target_link_libraries(vkEngine utils
                               glm)

set_target_properties(vkEngine PROPERTIES FOLDER "src")