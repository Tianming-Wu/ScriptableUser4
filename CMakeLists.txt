cmake_minimum_required(VERSION 3.10)

project(
    ScriptableUser4
    LANGUAGES CXX
    VERSION 1.0
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(SharedCppLib2 REQUIRED)

# if(WINDOWS)
    set(PLATFORM_SOURCE src/platform_windows.cpp)
# endif()

add_library(scapi STATIC src/scvar.cpp src/sctype.cpp)
target_link_libraries(scapi PUBLIC SharedCppLib2::basic)
target_include_directories(scapi PUBLIC "src")

add_executable(scuser
    src/main.cpp
    src/scuser.cpp
    src/internal_commands.cpp
    ${PLATFORM_SOURCE}
)
target_link_libraries(scuser SharedCppLib2::basic scapi)


add_library(windowsapi SHARED plugins/windowsapi.cpp)
target_link_libraries(windowsapi scapi)


add_library(maths SHARED plugins/maths.cpp)
target_link_libraries(maths scapi)


add_library(gdi SHARED plugins/gdi.cpp)
target_link_libraries(gdi scapi)



foreach(plugin_target IN ITEMS windowsapi maths gdi)
    set_target_properties(${plugin_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/scplugins"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/scplugins"
    )

    add_custom_command(TARGET ${plugin_target} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/scplugins"
    )
endforeach()