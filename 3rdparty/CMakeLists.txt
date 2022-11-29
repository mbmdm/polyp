cmake_minimum_required(VERSION 3.20)


#Building GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)


#Building GLAD
find_package(Python3 COMPONENTS Interpreter)
execute_process(COMMAND ${Python3_EXECUTABLE} -c "import jinja2" RESULT_VARIABLE res_var)
if(res_var EQUAL 0)
	message(STATUS "Python3 check passed.")
else()
	message(FATAL_ERROR "Python10 with module jinja2 is required!")
endif()
add_subdirectory(glad/cmake)
set(GLAD_BUILD_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/gladsources)
glad_add_library(glad_gl_core_46 STATIC LOCATION ${GLAD_BUILD_LOCATION} LANGUAGE C API gl:core=4.6)


#GLM
add_subdirectory(glm)
