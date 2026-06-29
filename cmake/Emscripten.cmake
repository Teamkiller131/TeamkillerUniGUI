# ── Emscripten (WebAssembly) third-party dependency setup ─────────────────────
#
# On desktop the project resolves imgui / implot / spdlog / freetype / glfw / glad via
# vcpkg find_package(). None of that applies to a wasm build: glad is a desktop-GL
# loader, and GLFW / WebGL2 / freetype are provided by Emscripten's own ports. So here
# we source the portable C++ libs from FetchContent and the GL/window/freetype stack
# from Emscripten link settings, then expose the SAME target names src/CMakeLists.txt
# expects (imgui::imgui, implot::implot, spdlog::spdlog, Freetype::Freetype, glad::glad).
#
# Included from the top-level CMakeLists.txt only when EMSCRIPTEN is set.

include(FetchContent)

# Emscripten port settings shared by every target that touches GL/GLFW/freetype.
# (SHELL: keeps the -s tokens from being de-duplicated/reordered by CMake.)
set(UNIGUI_EM_SETTINGS
    "SHELL:-sUSE_GLFW=3"
    "SHELL:-sUSE_WEBGL2=1"
    "SHELL:-sFULL_ES3=1"
    "SHELL:-sMIN_WEBGL_VERSION=2"
    "SHELL:-sMAX_WEBGL_VERSION=2"
    "SHELL:-sUSE_FREETYPE=1")

# ── spdlog (builds its own CMake target spdlog::spdlog) ───────────────────────
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "" FORCE)
# spdlog 1.15.x bundles fmt 11, which fixes a consteval bug that fmt 10 (shipped with
# spdlog 1.14) trips on Emscripten's Clang 19: "call to consteval function
# fmt::basic_format_string<…> is not a constant expression" in logger.h, failing every
# TU that includes <spdlog> (all of them, via core/log.h). fmt 10 ignored the
# FMT_USE_CONSTEVAL=0 escape hatch on that compiler, so the version bump is the real
# fix; the define stays as belt-and-suspenders (forces the constexpr ctor path). The
# project's format strings are already validated against fmt by the desktop builds.
FetchContent_Declare(spdlog_em
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3)
FetchContent_MakeAvailable(spdlog_em)
if(TARGET spdlog)
    target_compile_definitions(spdlog PUBLIC FMT_USE_CONSTEVAL=0)
endif()

# ── imgui (docking branch) — sources only, no upstream CMakeLists ─────────────
FetchContent_Declare(imgui_em
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.8-docking)
FetchContent_MakeAvailable(imgui_em)
add_library(unigui_imgui STATIC
    ${imgui_em_SOURCE_DIR}/imgui.cpp
    ${imgui_em_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_em_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_em_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_em_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_em_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp
    ${imgui_em_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_em_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(unigui_imgui PUBLIC
    ${imgui_em_SOURCE_DIR}
    ${imgui_em_SOURCE_DIR}/backends
    ${imgui_em_SOURCE_DIR}/misc/freetype)
target_compile_definitions(unigui_imgui PUBLIC IMGUI_ENABLE_FREETYPE)
# Emscripten provides GLFW3 headers + freetype headers when these settings are active.
target_compile_options(unigui_imgui PUBLIC ${UNIGUI_EM_SETTINGS})
# WebGPU build: compile imgui's wgpu backend and pull in the emdawnwebgpu port. imgui
# 1.92's imgui_impl_wgpu requires the modern webgpu.h (WGPUStringView, surface API, …)
# that `--use-port=emdawnwebgpu` provides — the legacy `-sUSE_WEBGPU=1` binding no longer
# compiles. The flag is needed at both compile (headers) and link (JS impl). The WebGL2
# build omits both.
if(UNIGUI_WEB_WEBGPU)
    target_sources(unigui_imgui PRIVATE ${imgui_em_SOURCE_DIR}/backends/imgui_impl_wgpu.cpp)
    target_compile_options(unigui_imgui PUBLIC "SHELL:--use-port=emdawnwebgpu")
endif()
add_library(imgui::imgui ALIAS unigui_imgui)

# ── implot — sources only, depends on imgui ───────────────────────────────────
# Must track imgui 1.92: the last implot tag (v0.16) still uses IM_OFFSETOF, which
# imgui 1.92 removed, so it fails to compile against our imgui. master switched to
# offsetof and matches the newer implot the desktop vcpkg build already uses.
FetchContent_Declare(implot_em
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG master)
FetchContent_MakeAvailable(implot_em)
add_library(unigui_implot STATIC
    ${implot_em_SOURCE_DIR}/implot.cpp
    ${implot_em_SOURCE_DIR}/implot_items.cpp)
target_include_directories(unigui_implot PUBLIC ${implot_em_SOURCE_DIR})
target_link_libraries(unigui_implot PUBLIC imgui::imgui)
add_library(implot::implot ALIAS unigui_implot)

# ── Freetype — provided by the emscripten port (no separate build) ────────────
add_library(unigui_freetype INTERFACE)
target_compile_options(unigui_freetype INTERFACE "SHELL:-sUSE_FREETYPE=1")
target_link_options(unigui_freetype INTERFACE "SHELL:-sUSE_FREETYPE=1")
add_library(Freetype::Freetype ALIAS unigui_freetype)

# ── glfw — provided by the emscripten port (the lib links the bare `glfw` target) ─
add_library(unigui_glfw INTERFACE)
target_compile_options(unigui_glfw INTERFACE "SHELL:-sUSE_GLFW=3")
target_link_options(unigui_glfw INTERFACE "SHELL:-sUSE_GLFW=3")
add_library(glfw ALIAS unigui_glfw)

# ── glad — unused on emscripten (GL is provided by the runtime) ───────────────
add_library(unigui_glad_stub INTERFACE)
add_library(glad::glad ALIAS unigui_glad_stub)

# Make the GLFW/WebGL/freetype link settings apply to the final wasm link.
add_link_options(${UNIGUI_EM_SETTINGS})
# WebGPU build also needs the emdawnwebgpu JS implementation linked in.
if(UNIGUI_WEB_WEBGPU)
    add_link_options("SHELL:--use-port=emdawnwebgpu")
endif()

message(STATUS "Emscripten dependency mode: imgui/implot/spdlog via FetchContent; "
               "GLFW/WebGL2/freetype via Emscripten ports; glad stubbed")
