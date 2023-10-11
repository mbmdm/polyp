cmake_minimum_required(VERSION 3.20)

#add_subdirectory(gl)
#add_subdirectory(vk)

#set(sources_vk main_vk.cpp)
#
#set(includes ${STB_LOCATION})
#
#include_directories(${includes})
#
#add_executable(example_vk ${sources_vk})
#
#if(MSVC)
#    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
##    set_property(TARGET example_gl PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
#    set_property(TARGET example_vk PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
#endif()
#
#target_link_libraries(example_vk glfw
#                                 glm
#                                 tools
#                                 vkEngine)
#
#set_target_properties(example_vk PROPERTIES FOLDER "samples")
