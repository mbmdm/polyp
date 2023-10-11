cmake_minimum_required(VERSION 3.20)

set(sources_vk main_vk.cpp)

set(includes ${STB_LOCATION})

include_directories(${includes})

add_executable(example_vk ${sources_vk})

target_link_libraries(example_vk glfw
                                 glm
                                 tools
                                 vkEngine)

set_target_properties(example_vk PROPERTIES FOLDER "samples/vk")
