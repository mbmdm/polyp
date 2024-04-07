cmake_minimum_required(VERSION 3.6)

set(sources camera.cpp
            polyp_window.cpp)

set(includes ${CMAKE_CURRENT_SOURCE_DIR}
             ${CMAKE_CURRENT_SOURCE_DIR}/interfaces)

set(headers camera.h
            constants.h
            polyp_window.h
            polyp_logs.h
            platforms.h
            interfaces/renderer.h)

set(sources ${sources} ${headers})

add_library(tools STATIC ${sources})

target_include_directories(tools PUBLIC ${includes})

target_link_libraries(tools glm)

set_target_properties(tools PROPERTIES FOLDER "src")
