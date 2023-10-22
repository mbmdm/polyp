cmake_minimum_required(VERSION 3.20)

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

function(buildSample SAMPLE_NAME)
    message(STATUS "Generating sample \"${SAMPLE_NAME}\"")
    SET(SAMPLE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/${SAMPLE_NAME})
    SET(MAIN_CPP ${SAMPLE_FOLDER}/${SAMPLE_NAME}.cpp)
    set(SHADER_DIR_GLSL "${SAMPLE_FOLDER}/shaders")
    file(GLOB SHADERS_GLSL "${SHADER_DIR_GLSL}/*.vert" "${SHADER_DIR_GLSL}/*.frag" "${SHADER_DIR_GLSL}/*.comp" "${SHADER_DIR_GLSL}/*.geom" "${SHADER_DIR_GLSL}/*.tesc" "${SHADER_DIR_GLSL}/*.tese" "${SHADER_DIR_GLSL}/*.mesh" "${SHADER_DIR_GLSL}/*.task" "${SHADER_DIR_GLSL}/*.rgen" "${SHADER_DIR_GLSL}/*.rchit" "${SHADER_DIR_GLSL}/*.rmiss" "${SHADER_DIR_GLSL}/*.rcall" "${SHADER_DIR_GLSL}/*.rahit" "${SHADER_DIR_GLSL}/*.rint" "${SHADER_DIR_GLSL}/*.glsl")
    foreach(GLSL ${SHADERS_GLSL})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV_DIR "${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)/shaders/${SAMPLE_NAME}")
        set(SPIRV "${SPIRV_DIR}/${FILE_NAME}.spv")
        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SPIRV_DIR}"
            COMMAND ${GLSL_VALIDATOR} --target-env vulkan1.2 -V ${GLSL} -o ${SPIRV}
            DEPENDS ${GLSL})
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach(GLSL)
    source_group("Shaders" FILES ${SHADERS_GLSL} ${SPIRV_BINARY_FILES})
    add_executable(${SAMPLE_NAME} ${MAIN_CPP} ${SHADERS_GLSL} ${SPIRV_BINARY_FILES})
    target_link_libraries(${SAMPLE_NAME} glfw glm tools vkEngine)
    set_target_properties(${SAMPLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$(Configuration)")
    #set_target_properties(${SAMPLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_target_properties(${SAMPLE_NAME} PROPERTIES FOLDER "samples/vk")
endfunction(buildSample)

function(buildSamples)
	foreach(SAMPLE ${SAMPLES})
		buildSample(${SAMPLE})
	endforeach(SAMPLE)
endfunction(buildSamples)

SUBDIRLIST(SAMPLES ${CMAKE_CURRENT_SOURCE_DIR})

buildSamples()
