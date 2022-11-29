cmake_minimum_required(VERSION 3.20)

set(sources main.cpp)

if(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
endif()

include_directories(${STB_LOCATION})
add_executable(example ${sources})
target_link_libraries(example glfw
							  glm
                              glad_gl_core_46)
