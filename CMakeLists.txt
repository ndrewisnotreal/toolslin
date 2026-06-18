cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif()

project(DBD_Tool VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

set(OpenGL_GL_PREFERENCE GLVND)

# Find dependencies
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Threads REQUIRED)
find_package(X11 REQUIRED)

if(TARGET glfw)
    set(DBD_TOOL_GLFW_TARGET glfw)
elseif(TARGET glfw3)
    set(DBD_TOOL_GLFW_TARGET glfw3)
else()
    message(FATAL_ERROR "glfw3 was found, but no known CMake target was exported.")
endif()

if(NOT TARGET X11::Xtst)
    message(FATAL_ERROR "X11 Xtst development files are required. Install libxtst-dev.")
endif()

# ImGui setup
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui")
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

foreach(IMGUI_SOURCE IN LISTS IMGUI_SOURCES)
    if(NOT EXISTS "${IMGUI_SOURCE}")
        message(FATAL_ERROR
            "Missing ImGui source file: ${IMGUI_SOURCE}\n"
            "Run: git submodule update --init --recursive"
        )
    endif()
endforeach()

# App sources
set(APP_SOURCES
    src/main.cpp
    src/memory.cpp
    src/ui.cpp
    src/esp.cpp
    src/globals.cpp
    src/classes.cpp
    src/math.cpp
    src/engine.cpp
    src/config.cpp
)

add_executable(${PROJECT_NAME} ${IMGUI_SOURCES} ${APP_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CMAKE_SOURCE_DIR}/vendor
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${DBD_TOOL_GLFW_TARGET}
    OpenGL::GL
    Threads::Threads
    ${CMAKE_DL_LIBS}
    X11::X11
    X11::Xtst
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Offsets.json"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Offsets.json"
    VERBATIM
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/Offsets.json"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
)
