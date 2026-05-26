cmake_minimum_required(VERSION 3.31)

project(unigui
    VERSION 3.1.0
    DESCRIPTION "Modular Dear ImGui C++ wrapper — core + backends + optional modules"
    HOMEPAGE_URL "https://xbw-nas.iepose.cn/Teamkiller131/TeamkillerUniGUI"
    LANGUAGES CXX
)

# ── Module Options ───────────────────────────────────────────────────────────
option(UNIGUI_MODULE_WIDGETS  "Extended widgets"     ON)
option(UNIGUI_MODULE_DSL       "Declarative UI DSL"   ON)
option(UNIGUI_MODULE_STYLING   "CSS styling engine"   ON)
option(UNIGUI_MODULE_FONTS     "Font manager"         ON)
option(UNIGUI_MODULE_EVENTS    "EventBus"             ON)
option(UNIGUI_MODULE_PLUGIN    "Plugin system"        ON)
option(UNIGUI_MODULE_SQLITE    "SQLite database"      OFF)
option(UNIGUI_MODULE_CONFIG    "TOML/JSON config"     OFF)
option(UNIGUI_MODULE_IPC       "Shared memory IPC"    OFF)
option(UNIGUI_MODULE_NETWORK   "HTTP/WebSocket"       OFF)
option(UNIGUI_BACKEND_DX11     "DX11 backend (Win)"   ON)
option(UNIGUI_BACKEND_DX12     "DX12 backend (Win)"    OFF)
option(UNIGUI_BACKEND_GLFW3    "GLFW+OpenGL3 backend" ON)
option(UNIGUI_BUILD_SHARED     "Build as DLL"         OFF)
option(UNIGUI_BUILD_TESTS      "Build test suite"     ON)
option(UNIGUI_BUILD_EXAMPLES   "Build examples"       ON)

# ── Subdirectories ───────────────────────────────────────────────────────────
add_subdirectory(src)

if(UNIGUI_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(UNIGUI_BUILD_EXAMPLES)
    add_subdirectory(examples/hello_unigui)
    if(UNIGUI_MODULE_WIDGETS)
        add_subdirectory(examples/widget_gallery)
        add_subdirectory(examples/v3_overview)
        add_subdirectory(examples/theme_demo)
    endif()
    add_subdirectory(examples/form_demo)
    add_subdirectory(examples/plot_demo)
    if(UNIGUI_MODULE_PLUGIN)
        add_subdirectory(examples/plugin_example)
    endif()
    if(UNIGUI_MODULE_DSL)
        add_subdirectory(examples/hello_unigui_v2)
    endif()
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
