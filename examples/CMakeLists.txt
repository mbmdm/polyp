cmake_minimum_required(VERSION 3.20)

set(sources_gl main_gl.cpp)
set(sources_vk main_Vk.cpp)

set(includes ${STB_LOCATION})

message(STATUS "ANISLOG: ${includes}")

set(shaders shaders/example.fs
            shaders/example.vs)

include_directories(${includes})

add_executable(example_gl ${sources_gl} 
                          ${shaders})
add_executable(example_vk ${sources_vk} 
                          ${shaders})

if(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
    set_property(TARGET example_gl PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    set_property(TARGET example_vk PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

foreach(shader ${shaders})
    add_custom_command(TARGET example_gl PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${shader} $<TARGET_FILE_DIR:example_gl>/${shader})
	add_custom_command(TARGET example_vk PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${shader} $<TARGET_FILE_DIR:example_vk>/${shader})
endforeach(shader)

target_link_libraries(example_vk glfw
                                 glm
                                 utils
                                 vkEngine)
target_link_libraries(example_gl glfw
                                 glm
                                 utils
                                 glEngine
                                 glad_gl_core_46)

set_target_properties(example_vk PROPERTIES FOLDER "examples")
set_target_properties(example_gl PROPERTIES FOLDER "examples")
