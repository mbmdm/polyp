cmake_minimum_required(VERSION 3.20)

project(Polyp)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
set(POLYP_ROOT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src" CACHE PATH "Root src path")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
    add_compile_definitions(NOMINMAX)
    if ( MSVC )
        set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} /wd26812")
    endif()
    # /ENTRY:mainCRTStartup keeps the same "main" function instead of requiring "WinMain"
    set(SUBSYSTEM_LINKER_OPTIONS "$<$<CONFIG:RELEASE>:/SUBSYSTEM:WINDOWS>"
                                 "$<$<CONFIG:RELEASE>:/ENTRY:mainCRTStartup>")
endif()

add_compile_options("$<$<CONFIG:DEBUG>:-DDEBUG>")

add_subdirectory(3rdparty)
add_subdirectory(samples)
add_subdirectory(src)
