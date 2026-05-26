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
find_package(implot CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(UNIGUI_LIBS
    imgui::imgui
    implot::implot
    glfw
    glad::glad
    Freetype::Freetype
    spdlog::spdlog
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

# ── DX11 + DX12 (Windows only, always available via vcpkg) ────────────────────
if(WIN32)
    list(APPEND UNIGUI_LIBS d3d11 d3dcompiler dxgi dxguid d3d12)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_DX11)
    if(UNIGUI_BACKEND STREQUAL "DX12")
        target_compile_definitions(unigui PUBLIC UNIGUI_HAS_DX12)
    endif()
    message(STATUS "DX11 + DX12 backends available")
endif()

target_link_libraries(unigui PUBLIC ${UNIGUI_LIBS})

# ── Sources (added incrementally per task) ───────────────────────────────────
target_sources(unigui PRIVATE
    unigui.cc
    core/context.cc
    core/error.cc
    core/locale.cc
    core/settings.cc
    core/log.cc
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
    widgets/datepicker.cc
    widgets/image.cc
    widgets/loadingindicator.cc
    widgets/notification.cc
    widgets/hyperlink.cc
    widgets/tag.cc
    widgets/breadcrumb.cc
    widgets/multiline.cc
    widgets/iconbutton.cc
    widgets/space.cc
    widgets/richtext.cc
    widgets/imagebutton.cc
    widgets/markdown.cc
    widgets/virtuallist.cc
    widgets/multicombo.cc
    widgets/propertygrid.cc
    widgets/searchbox.cc
    widgets/toast.cc
    widgets/trayicon.cc
    widgets/passwordinput.cc
    widgets/wizard.cc
    app/app.cc
    v2/plugin_manager.cc
    v2/dsl.cc
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
    backend/webgpu_renderer.cc
    backend/emscripten_platform.cc
)
    # DX12 only when explicitly requested
    if(UNIGUI_BACKEND STREQUAL "DX12")
        target_sources(unigui PRIVATE backend/dx12_renderer.cc)
    endif()
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

# ── Font embedding ──────────────────────────────────────────────────────────
include(${CMAKE_SOURCE_DIR}/cmake/FontEmbed.cmake)
set(FONT_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/fonts)
file(MAKE_DIRECTORY ${FONT_GEN_DIR})
embed_font(unigui
    ${CMAKE_SOURCE_DIR}/fonts/JetBrainsMonoNerdFont-Regular.ttf
    ${FONT_GEN_DIR}/jb_mono_font.h
    jb_mono
)
target_include_directories(unigui PRIVATE ${FONT_GEN_DIR})

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
