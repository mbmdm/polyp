cmake_minimum_required(VERSION 3.20)

set(sources main.cpp)

set(includes ${STB_LOCATION}
             utils
             glEngine
             vkEngine)

set(shaders shaders/example.fs
            shaders/example.vs)

include_directories(${includes})

add_executable(example ${sources} 
                       ${shaders})
							  
if(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
    set_property(TARGET example PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

foreach(shader ${shaders})
    add_custom_command(TARGET example PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${shader} $<TARGET_FILE_DIR:example>/${shader})
endforeach(shader)

target_link_libraries(example glfw
                              glm
                              utils
                              glEngine
                              vkEngine
                              glad_gl_core_46)
