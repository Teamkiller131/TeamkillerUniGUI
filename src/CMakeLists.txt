# ── Library target ───────────────────────────────────────────────────────────
add_library(unigui "")
add_library(unigui::unigui ALIAS unigui)

# Respect global BUILD_SHARED_LIBS
set_target_properties(unigui PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# ── Export header (DLL import/export macros) ─────────────────────────────────
include(GenerateExportHeader)
generate_export_header(unigui
    EXPORT_MACRO_NAME UNIGUI_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/unigui_export.h
)

# ── Dependencies (vcpkg) ─────────────────────────────────────────────────────
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(Freetype REQUIRED)

set(UNIGUI_LIBS
    imgui::imgui
    glfw
    glad::glad
    Freetype::Freetype
)

# ── SDL3 + Vulkan (conditional) ───────────────────────────────────────────────
if(UNIGUI_BACKEND STREQUAL "SDL3_VULKAN")
    find_package(SDL3 CONFIG REQUIRED)
    find_package(Vulkan REQUIRED)
    list(APPEND UNIGUI_LIBS SDL3::SDL3 Vulkan::Vulkan)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_SDL3_VULKAN)
    message(STATUS "Building with SDL3 + Vulkan backend")
else()
    message(STATUS "Building with GLFW + OpenGL3 backend")
endif()

# ── DX11 (Windows only, always available via vcpkg) ───────────────────────────
if(WIN32)
    list(APPEND UNIGUI_LIBS d3d11 d3dcompiler dxgi dxguid)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_DX11)
    message(STATUS "DX11 backend available")
endif()

target_link_libraries(unigui PUBLIC ${UNIGUI_LIBS})

# ── Sources (added incrementally per task) ───────────────────────────────────
target_sources(unigui PRIVATE
    unigui.cc
    core/context.cc
    core/error.cc
    backend/glfw_platform.cc
    backend/opengl3_renderer.cc
    backend/backend_registry.cc
    theme/theme.cc
    theme/style_scope.cc
    widgets/widget_base.cc
    widgets/label.cc
    widgets/button.cc
    widgets/panel.cc
    widgets/form.cc
    widgets/window.cc
    widgets/checkbox.cc
    widgets/progressbar.cc
    widgets/radiogroup.cc
    widgets/combobox.cc
    widgets/lineedit.cc
    widgets/groupbox.cc
    widgets/tabwidget.cc
    widgets/treeview.cc
    widgets/listview.cc
    widgets/dialog.cc
    widgets/menubar.cc
    widgets/statusbar.cc
    widgets/toolbar.cc
    widgets/table.cc
    widgets/colorpicker.cc
    widgets/filepath.cc
    widgets/dirpath.cc
    widgets/toggleswitch.cc
    widgets/inputint.cc
    widgets/inputfloat.cc
    widgets/splitter.cc
    widgets/separator.cc
    widgets/scrollarea.cc
    app/app.cc
)

# ── SDL3 + Vulkan sources (conditional) ───────────────────────────────────────
if(UNIGUI_BACKEND STREQUAL "SDL3_VULKAN")
    target_sources(unigui PRIVATE
        backend/sdl3_platform.cc
        backend/vulkan_device.cc
        backend/vulkan_swapchain.cc
        backend/vulkan_pipeline.cc
        backend/vulkan_frame.cc
        backend/vulkan_renderer.cc
        backend/vulkan_context.cc
    )
endif()

# ── DX11 + Metal (Windows) ──────────────────────────────────────────────────
if(WIN32)
    target_sources(unigui PRIVATE
        backend/dx11_renderer.cc
        backend/metal_renderer.cc
    )
endif()

# ── Include directories ──────────────────────────────────────────────────────
target_include_directories(unigui
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ── Install ───────────────────────────────────────────────────────────────────
install(TARGETS unigui
    EXPORT unigui-targets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/unigui_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/unigui
)
