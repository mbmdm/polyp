cmake_minimum_required(VERSION 3.6)

set(sources ${CMAKE_CURRENT_SOURCE_DIR}/camera.cpp)

set(includes ${CMAKE_CURRENT_SOURCE_DIR})
	
add_library(utils STATIC ${sources})

target_include_directories(utils PUBLIC ${includes})

target_link_libraries(utils glm
                            glad_gl_core_46)
