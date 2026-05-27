# ── Library target ───────────────────────────────────────────────────────────
add_library(unigui "")
add_library(unigui::unigui ALIAS unigui)

# CRT diagnostic for MSVC users
if(MSVC)
    if(CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "MultiThreadedDLL")
        message(STATUS "UniGUI CRT: /MD (Dynamic)")
    elseif(CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "MultiThreaded\$")
        message(STATUS "UniGUI CRT: /MT (Static)")
    else()
        message(WARNING "UniGUI: Unknown MSVC runtime library: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
    endif()
endif()

set_target_properties(unigui PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# ── Export header ─────────────────────────────────────────────────────────────
include(GenerateExportHeader)
generate_export_header(unigui
    EXPORT_MACRO_NAME UNIGUI_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/unigui_export.h
)

# ── Dependencies (always) ─────────────────────────────────────────────────────
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

# ── Optional module deps ──────────────────────────────────────────────────────
if(UNIGUI_MODULE_SQLITE)
    find_package(SQLite3 REQUIRED)
    list(APPEND UNIGUI_LIBS SQLite::SQLite3)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_SQLITE)
endif()
if(UNIGUI_MODULE_IPC)
    find_package(ZeroMQ REQUIRED)
    list(APPEND UNIGUI_LIBS libzmq)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_IPC)
endif()

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

# ── DX11 + DX12 (Windows, optional) ───────────────────────────────────────────
if(WIN32)
    if(UNIGUI_BACKEND_DX11)
        list(APPEND UNIGUI_LIBS d3d11 d3dcompiler dxgi dxguid)
        target_compile_definitions(unigui PUBLIC UNIGUI_HAS_DX11)
        message(STATUS "DX11 backend enabled")
    endif()
    if(UNIGUI_BACKEND_DX12)
        list(APPEND UNIGUI_LIBS d3d12)
        target_compile_definitions(unigui PUBLIC UNIGUI_HAS_DX12)
        message(STATUS "DX12 backend enabled")
    endif()
endif()

# ── macOS-specific: Objective-C++ flags for Metal backend ──────────────────────
if(APPLE)
    set_source_files_properties(backend/metal_renderer.cc PROPERTIES
        COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
    find_library(METAL_LIBRARY Metal REQUIRED)
    find_library(QUARTZCORE_LIBRARY QuartzCore REQUIRED)
    list(APPEND UNIGUI_LIBS ${METAL_LIBRARY} ${QUARTZCORE_LIBRARY})
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_METAL)
    message(STATUS "Metal backend enabled (macOS)")
endif()

target_link_libraries(unigui PUBLIC ${UNIGUI_LIBS})

# ═══════════════════════════════════════════════════════════════════════════════
# Sources — conditional per module
# ═══════════════════════════════════════════════════════════════════════════════

# ── Core sources (always compiled) ────────────────────────────────────────────
target_sources(unigui PRIVATE
    unigui.cc
    core/context.cc
    core/error.cc
    core/locale.cc
    core/settings.cc
    core/log.cc
    core/main_thread.cc
    backend/glfw_platform.cc
    backend/opengl3_renderer.cc
    backend/backend_registry.cc
    theme/theme.cc
    theme/style_scope.cc
    app/app.cc
    # Core widgets — fundamental UI primitives
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
    widgets/separator.cc
    widgets/scrollarea.cc
    widgets/dialog.cc
    widgets/menubar.cc
    widgets/statusbar.cc
    widgets/toolbar.cc
)

# ── Extended widgets (UNIGUI_MODULE_WIDGETS) ──────────────────────────────────
if(UNIGUI_MODULE_WIDGETS)
    target_sources(unigui PRIVATE
        widgets/treeview.cc
        widgets/listview.cc
        widgets/table.cc
        widgets/colorpicker.cc
        widgets/filepath.cc
        widgets/dirpath.cc
        widgets/toggleswitch.cc
        widgets/inputint.cc
        widgets/inputfloat.cc
        widgets/splitter.cc
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
        widgets/card.cc
        widgets/shimmer.cc
        widgets/badge.cc
        widgets/skeleton.cc
        widgets/herosection.cc
        widgets/multihandleslider.cc
        widgets/timeseries_chart.cc
        fonts/gradient_text.cc
    )
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_WIDGETS)
    message(STATUS "Extended widgets enabled")
endif()

# ── v2 modules (optional) ─────────────────────────────────────────────────────
if(UNIGUI_MODULE_DSL)
    target_sources(unigui PRIVATE v2/dsl.cc)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_DSL)
    message(STATUS "DSL module enabled")
endif()
if(UNIGUI_MODULE_STYLING)
    target_sources(unigui PRIVATE v2/style_engine.cc)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_STYLING)
    message(STATUS "Styling module enabled")
endif()
if(UNIGUI_MODULE_FONTS)
    target_sources(unigui PRIVATE v2/font_manager.cc)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_FONTS)
    message(STATUS "Font manager enabled")
endif()
if(UNIGUI_MODULE_EVENTS)
    target_sources(unigui PRIVATE v2/eventbus.cc)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_EVENTS)
    message(STATUS "EventBus enabled")
endif()
if(UNIGUI_MODULE_PLUGIN)
    target_sources(unigui PRIVATE v2/plugin_manager.cc)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_PLUGIN)
    message(STATUS "Plugin system enabled")
endif()
if(UNIGUI_MODULE_SQLITE)
    target_sources(unigui PRIVATE v2/database.cc)
    message(STATUS "SQLite module enabled")
endif()
if(UNIGUI_MODULE_CONFIG)
    target_sources(unigui PRIVATE v2/config.cc)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_CONFIG)
    message(STATUS "Config module enabled")
endif()
if(UNIGUI_MODULE_IPC)
    target_sources(unigui PRIVATE v2/shmem.cc)
    message(STATUS "IPC module enabled")
endif()

# ── fx effects (core — always compiled) ─────────────────────────────────────
target_sources(unigui PRIVATE     fx/effect_scope.cc fx/animation.cc
    # theme registry
    theme/theme_registry.cc)

# ── Backend sources ───────────────────────────────────────────────────────────
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

if(WIN32)
    target_sources(unigui PRIVATE
        backend/metal_renderer.cc
        backend/webgpu_renderer.cc
        backend/emscripten_platform.cc
        backend/dx11_renderer.cc
    )
    if(UNIGUI_BACKEND_DX12)
        target_sources(unigui PRIVATE backend/dx12_renderer.cc)
    endif()
else()
    # Linux/macOS: include stubs needed by backend_factory.h
    target_sources(unigui PRIVATE
        backend/webgpu_renderer.cc
        backend/emscripten_platform.cc
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
