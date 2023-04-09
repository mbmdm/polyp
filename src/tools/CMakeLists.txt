cmake_minimum_required(VERSION 3.6)

set(sources camera.cpp
            error_codes.cpp
            window_surface.cpp)

set(includes ${CMAKE_CURRENT_SOURCE_DIR}
             ${CMAKE_CURRENT_SOURCE_DIR}/interfaces)

set(headers camera.h
            constants.h
            error_codes.h
            window_surface.h
            platforms.h
            polyp_logs.h
            interfaces/renderer.h)

set(sources ${sources} ${headers})

add_library(tools STATIC ${sources})

target_include_directories(tools PUBLIC ${includes})

target_link_libraries(tools glm)

set_target_properties(tools PROPERTIES FOLDER "src")
