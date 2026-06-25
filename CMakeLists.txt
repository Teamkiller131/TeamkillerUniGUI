cmake_minimum_required(VERSION 3.26)

# ── Version (single source of truth: include/unigui/core/version.h) ────────────
# Derive the project version from version.h so the package config-version file,
# vcpkg port, and the C++ macros can never drift apart.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/include/unigui/core/version.h" _unigui_version_h)
string(REGEX MATCH "#define UNIGUI_VERSION_MAJOR ([0-9]+)" _ "${_unigui_version_h}")
set(_unigui_vmajor "${CMAKE_MATCH_1}")
string(REGEX MATCH "#define UNIGUI_VERSION_MINOR ([0-9]+)" _ "${_unigui_version_h}")
set(_unigui_vminor "${CMAKE_MATCH_1}")
string(REGEX MATCH "#define UNIGUI_VERSION_PATCH ([0-9]+)" _ "${_unigui_version_h}")
set(_unigui_vpatch "${CMAKE_MATCH_1}")
unset(_unigui_version_h)

project(unigui
    VERSION ${_unigui_vmajor}.${_unigui_vminor}.${_unigui_vpatch}
    DESCRIPTION "Modular Dear ImGui C++ wrapper — core + backends + optional modules"
    HOMEPAGE_URL "https://github.com/Teamkiller131/TeamkillerUniGUI"
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
option(UNIGUI_MODULE_TRADING   "Trading-client toolkit (models + widgets)" OFF)
option(UNIGUI_BACKEND_DX11     "DX11 backend (Win)"   ON)
option(UNIGUI_BACKEND_DX12     "DX12 backend (Win)"    OFF)
option(UNIGUI_BACKEND_VULKAN   "Vulkan renderer via GLFW (cross-platform)" OFF)
option(UNIGUI_BACKEND_SDL3     "SDL3 platform backend (opt-in; needs sdl3+imgui[sdl3-binding]+vulkan)" OFF)
option(UNIGUI_BACKEND_GLFW3    "GLFW+OpenGL3 backend" ON)
option(UNIGUI_BUILD_SHARED     "Build as DLL"         OFF)
option(UNIGUI_BUILD_TESTS      "Build test suite"     ON)
option(UNIGUI_BUILD_EXAMPLES   "Build examples"       ON)
option(UNIGUI_WARNINGS_AS_ERRORS "Treat compiler warnings as errors (opt-in)" OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Export compile_commands.json for clang-tidy/IDE")

# ── Warning configuration ────────────────────────────────────────────────────
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerWarnings.cmake)

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
        add_subdirectory(examples/unigui_showcase)
        add_subdirectory(examples/v3_overview)
        add_subdirectory(examples/theme_demo)
        add_subdirectory(examples/theme_editor)
    endif()
    add_subdirectory(examples/form_demo)
    add_subdirectory(examples/plot_demo)
    if(UNIGUI_MODULE_WIDGETS AND UNIGUI_MODULE_TRADING)
        add_subdirectory(examples/trading_dashboard)
    endif()
    if(UNIGUI_MODULE_PLUGIN AND WIN32)
        add_subdirectory(examples/plugin_example)
    endif()
    if(UNIGUI_MODULE_DSL)
        add_subdirectory(examples/hello_unigui_v2)
    endif()
    if(UNIGUI_BACKEND_SDL3)
        add_subdirectory(examples/vulkan_triangle)
    endif()
endif()

# ── Installation ─────────────────────────────────────────────────────────────
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Emit find_dependency() calls into the installed package config so that
# `find_package(unigui)` re-resolves the public dependencies referenced by the
# exported target's INTERFACE_LINK_LIBRARIES. Without this, a downstream
# consumer fails at generate time with "target links to imgui::imgui that does
# not exist". The set mirrors src/CMakeLists.txt's find_package() calls exactly
# (same names + CONFIG/module mode), gated by the same options.
set(UNIGUI_DEP_CALLS "")
macro(_unigui_add_dep)
    string(REPLACE ";" " " _unigui_dep_args "${ARGN}")
    string(APPEND UNIGUI_DEP_CALLS "find_dependency(${_unigui_dep_args})\n")
endmacro()
_unigui_add_dep(imgui CONFIG)
_unigui_add_dep(glfw3 CONFIG)
_unigui_add_dep(glad CONFIG)
_unigui_add_dep(Freetype)
_unigui_add_dep(implot CONFIG)
_unigui_add_dep(spdlog CONFIG)
if(UNIGUI_MODULE_SQLITE)
    _unigui_add_dep(SQLite3)
endif()
if(UNIGUI_MODULE_CONFIG)
    _unigui_add_dep(cpptoml CONFIG)
    _unigui_add_dep(nlohmann_json CONFIG)
endif()
if(UNIGUI_MODULE_IPC)
    _unigui_add_dep(ZeroMQ CONFIG)
endif()
if(UNIGUI_MODULE_NETWORK)
    _unigui_add_dep(httplib CONFIG)
    _unigui_add_dep(ixwebsocket CONFIG)
endif()
if(UNIGUI_BACKEND_VULKAN OR UNIGUI_BACKEND_SDL3)
    _unigui_add_dep(Vulkan)
endif()
if(UNIGUI_BACKEND_SDL3)
    _unigui_add_dep(SDL3 CONFIG)
endif()

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

# ═══════════════════════════════════════════════════════════════════════════════
# Lint — run clang-tidy on all UniGUI sources (standalone, outside build)
# ═══════════════════════════════════════════════════════════════════════════════
find_program(CLANG_TIDY_EXE clang-tidy)
if(CLANG_TIDY_EXE)
    # Collect all UniGUI source files for linting
    file(GLOB_RECURSE UNIGUI_SOURCES
        "${PROJECT_SOURCE_DIR}/src/*.cc"
        "${PROJECT_SOURCE_DIR}/include/unigui/*.h"
        "${PROJECT_SOURCE_DIR}/include/unigui/**/*.h"
    )

    # Only lint tests if built
    if(UNIGUI_BUILD_TESTS)
        file(GLOB_RECURSE UNIGUI_TEST_SOURCES "${PROJECT_SOURCE_DIR}/tests/*.cc")
        list(APPEND UNIGUI_SOURCES ${UNIGUI_TEST_SOURCES})
    endif()

    add_custom_target(lint
        COMMENT "Running clang-tidy on ${PROJECT_NAME} sources..."
        COMMAND ${CLANG_TIDY_EXE}
            -p ${CMAKE_BINARY_DIR}
            --config-file=${PROJECT_SOURCE_DIR}/.clang-tidy
            ${UNIGUI_SOURCES}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        USES_TERMINAL
    )
    message(STATUS "Lint target available: cmake --build <dir> --target lint")
endif()

# ═══════════════════════════════════════════════════════════════════════════════
# Coverage — run tests + generate HTML coverage report (clang source-based)
# ═══════════════════════════════════════════════════════════════════════════════
find_program(LLVM_PROFDATA_EXE llvm-profdata)
find_program(LLVM_COV_EXE llvm-cov)
if(LLVM_PROFDATA_EXE AND LLVM_COV_EXE)
    set(COVERAGE_DIR "${CMAKE_BINARY_DIR}/coverage")
    set(COVERAGE_PROFRAW "${COVERAGE_DIR}/unigui-%p.profraw")
    set(COVERAGE_PROFDATA "${COVERAGE_DIR}/unigui.profdata")

    add_custom_target(coverage
        COMMENT "Collecting coverage data..."
        # Step 1: Run tests with LLVM_PROFILE_FILE set
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_DIR}
        COMMAND ${CMAKE_COMMAND} -E env
            "LLVM_PROFILE_FILE=${COVERAGE_PROFRAW}"
            ${CMAKE_CTEST_COMMAND}
                --test-dir ${CMAKE_BINARY_DIR}
                --output-on-failure
        # Step 2: Merge raw profiles
        COMMAND ${LLVM_PROFDATA_EXE} merge -sparse
            ${COVERAGE_DIR}/*.profraw
            -o ${COVERAGE_PROFDATA}
        # Step 3: Generate HTML report
        COMMAND ${LLVM_COV_EXE} show
            -format=html
            -output-dir=${COVERAGE_DIR}/html
            -instr-profile=${COVERAGE_PROFDATA}
            $<TARGET_FILE:unigui>
            -show-line-counts-or-regions
            -show-instantiation-summary
        # Step 4: Generate text summary
        COMMAND ${LLVM_COV_EXE} report
            -instr-profile=${COVERAGE_PROFDATA}
            $<TARGET_FILE:unigui>
        COMMENT "Coverage report: ${COVERAGE_DIR}/html/index.html"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        USES_TERMINAL
    )
    message(STATUS "Coverage target available: cmake --build <dir> --target coverage")
else()
    message(STATUS "Coverage target unavailable: install llvm-profdata + llvm-cov")
endif()
