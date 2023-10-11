cmake_minimum_required(VERSION 3.20)

set(sources_gl main_gl.cpp)

set(includes ${STB_LOCATION})

set(shaders shaders/example.fs
            shaders/example.vs)

include_directories(${includes})

add_executable(example_gl ${sources_gl} 
                          ${shaders})

if(MSVC)
    set_property(TARGET example_gl PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

foreach(shader ${shaders})
    add_custom_command(TARGET example_gl PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${shader} $<TARGET_FILE_DIR:example_gl>/${shader})
endforeach(shader)

target_link_libraries(example_gl glfw
                                 glm
                                 tools
                                 glEngine
                                 glad_gl_core_46)

set_target_properties(example_gl PROPERTIES FOLDER "samples/gl")

#add_subdirectory(gl)
