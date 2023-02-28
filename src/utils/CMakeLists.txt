cmake_minimum_required(VERSION 3.6)

set(sources ${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp
            utils_errors.cpp)

set(includes ${CMAKE_CURRENT_SOURCE_DIR})

set(headers camera.h
            constants.h
            utils_errors.h)

set(sources ${sources} ${headers})

add_library(utils STATIC ${sources})

target_include_directories(utils PUBLIC ${includes})

target_link_libraries(utils glm)

set_target_properties(utils PROPERTIES FOLDER "src")
