cmake_minimum_required(VERSION 3.31)

project(unigui
    VERSION 1.0.0
    DESCRIPTION "Modern Dear ImGui C++ wrapper — SDL3+Vulkan + GLFW+OpenGL3 backends, theme engine, widget library"
    HOMEPAGE_URL "https://github.com/Teamkiller131/TeamkillerUniGUI"
    LANGUAGES CXX
)

# ── Options ──────────────────────────────────────────────────────────────────
option(UNIGUI_BUILD_SHARED "Build unigui as a shared library (DLL)" OFF)
option(UNIGUI_BUILD_TESTS   "Build the Google Test test suite"        ON)
option(UNIGUI_BUILD_EXAMPLES "Build example applications"             ON)
set(UNIGUI_BACKEND "GLFW_GL3" CACHE STRING "Backend to build: GLFW_GL3 or SDL3_VULKAN")
set_property(CACHE UNIGUI_BACKEND PROPERTY STRINGS "GLFW_GL3" "SDL3_VULKAN")

# ── Subdirectories ───────────────────────────────────────────────────────────
add_subdirectory(src)

if(UNIGUI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(UNIGUI_BUILD_EXAMPLES)
    add_subdirectory(examples/hello_unigui)
    add_subdirectory(examples/widget_gallery)
    add_subdirectory(examples/form_demo)
    add_subdirectory(examples/plot_demo)
    if(UNIGUI_BACKEND STREQUAL "SDL3_VULKAN")
        add_subdirectory(examples/vulkan_triangle)
    endif()
endif()

# ── Installation ─────────────────────────────────────────────────────────────
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

configure_package_config_file(
    cmake/unigui-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/unigui-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unigui
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/unigui-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/unigui-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/unigui-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unigui
)

install(EXPORT unigui-targets
    FILE unigui-targets.cmake
    NAMESPACE unigui::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/unigui
)
