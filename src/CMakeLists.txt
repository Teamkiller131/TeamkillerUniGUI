# ── Library target ───────────────────────────────────────────────────────────
add_library(unigui "")
add_library(unigui::unigui ALIAS unigui)

# Windows: force the lean <windows.h> everywhere and kill the min/max macros. Several
# headers pull in <windows.h>; without WIN32_LEAN_AND_MEAN it drags in the legacy
# <winsock.h>, which then collides with the <winsock2.h>/<ws2tcpip.h> that the network
# module's httplib/ixwebsocket need (the umbrella TU mixes both). PUBLIC so consumers
# that aggregate our headers inherit the same safe defaults.
if(WIN32)
    target_compile_definitions(unigui PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

# CRT diagnostic for MSVC integration users
if(MSVC)
    if(CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "MultiThreadedDLL")
        message(STATUS "UniGUI CRT: /MD (Dynamic)")
    elseif(CMAKE_MSVC_RUNTIME_LIBRARY MATCHES "MultiThreaded$")
        message(STATUS "UniGUI CRT: /MT (Static)")
    else()
        message(WARNING "UniGUI: Unknown MSVC runtime: ${CMAKE_MSVC_RUNTIME_LIBRARY}")
    endif()
endif()

set_target_properties(unigui PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Base compiler warnings (always on); -Werror/-WX gated by UNIGUI_WARNINGS_AS_ERRORS.
unigui_set_warnings(unigui)

# ── Export header ─────────────────────────────────────────────────────────────
include(GenerateExportHeader)
generate_export_header(unigui
    EXPORT_MACRO_NAME UNIGUI_API
    EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/unigui_export.h
)

# ── Dependencies (always) ─────────────────────────────────────────────────────
# On Emscripten these targets are pre-created by cmake/Emscripten.cmake (FetchContent
# + Emscripten ports), so the vcpkg find_package() path is skipped — but the SAME
# target names (imgui::imgui, implot::implot, glfw, glad::glad, Freetype::Freetype,
# spdlog::spdlog) exist, so everything below links unchanged.
if(NOT EMSCRIPTEN)
    find_package(imgui CONFIG REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glad CONFIG REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(implot CONFIG REQUIRED)
    find_package(spdlog CONFIG REQUIRED)
endif()

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
    list(APPEND UNIGUI_LIBS SQLite3::SQLite3)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_SQLITE)
endif()
if(UNIGUI_MODULE_CONFIG)
    find_package(cpptoml CONFIG REQUIRED)
    find_package(nlohmann_json CONFIG REQUIRED)
    # cpptoml's vcpkg port exports an unnamespaced INTERFACE target `cpptoml`
    # (not `cpptoml::cpptoml`).
    list(APPEND UNIGUI_LIBS cpptoml nlohmann_json::nlohmann_json)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_CONFIG)
endif()
if(UNIGUI_MODULE_IPC)
    find_package(ZeroMQ CONFIG REQUIRED)
    list(APPEND UNIGUI_LIBS libzmq)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_IPC)
endif()
if(UNIGUI_MODULE_NETWORK)
    find_package(httplib CONFIG REQUIRED)
    find_package(ixwebsocket CONFIG REQUIRED)
    list(APPEND UNIGUI_LIBS httplib::httplib ixwebsocket::ixwebsocket)
    # ixwebsocket's TLS backend (mbedtls) calls BCryptGenRandom, which lives in
    # bcrypt.lib; the vcpkg port does not propagate it as a transitive link dep.
    if(WIN32)
        list(APPEND UNIGUI_LIBS bcrypt)
    endif()
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_NETWORK)
endif()

# ── Vulkan renderer (cross-platform) ──────────────────────────────────────────
# A single, platform-agnostic Vulkan renderer is shared by the GLFW Vulkan backend
# (UNIGUI_BACKEND_VULKAN, any OS) and the opt-in SDL3 backend (UNIGUI_BACKEND_SDL3).
# The only OS-specific step — surface creation — is delegated to the active
# PlatformBackend, so the renderer itself links nothing beyond Vulkan.
set(_UNIGUI_NEED_VULKAN OFF)
if(UNIGUI_BACKEND_VULKAN OR UNIGUI_BACKEND_SDL3)
    set(_UNIGUI_NEED_VULKAN ON)
endif()
if(_UNIGUI_NEED_VULKAN)
    find_package(Vulkan REQUIRED)
    list(APPEND UNIGUI_LIBS Vulkan::Vulkan)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_VULKAN)
    message(STATUS "Vulkan renderer enabled (cross-platform; surface via platform backend)")
endif()

# ── SDL3 platform backend (opt-in: needs sdl3 + imgui[sdl3-binding] + vulkan) ──
# Off by default. UniGUI is a backend-abstraction layer, so the SDL3 code is kept
# in-tree but only compiled/used when explicitly requested AND its dependencies are
# present. Pairs with the shared Vulkan renderer (SDL3_Vulkan backend).
if(UNIGUI_BACKEND_SDL3)
    find_package(SDL3 CONFIG REQUIRED)
    list(APPEND UNIGUI_LIBS SDL3::SDL3)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_SDL3)
    message(STATUS "SDL3 platform backend enabled (opt-in)")
endif()

# ── DX11 + DX12 (Windows, optional) ───────────────────────────────────────────
if(WIN32)
    if(UNIGUI_BACKEND_DX11)
        list(APPEND UNIGUI_LIBS d3d11 d3dcompiler dxgi dxguid)
        target_compile_definitions(unigui PUBLIC UNIGUI_HAS_DX11)
        message(STATUS "DX11 backend enabled")
    endif()
    if(UNIGUI_BACKEND_DX12)
        list(APPEND UNIGUI_LIBS d3d12 dxgi dxguid)
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
    find_library(COCOA_LIBRARY Cocoa REQUIRED) # NSWindow/NSView for the CAMetalLayer
    list(APPEND UNIGUI_LIBS ${METAL_LIBRARY} ${QUARTZCORE_LIBRARY} ${COCOA_LIBRARY})
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
    core/strutil.cc
    core/log.cc
    core/main_thread.cc
    core/accessibility.cc
    backend/glfw_platform.cc
    backend/opengl3_renderer.cc
    backend/backend_registry.cc
    theme/theme.cc
    theme/style_scope.cc
    app/app.cc
    im/im.cc
    # Core widgets — fundamental UI primitives
    widgets/widget_base.cc
    widgets/label.cc
    widgets/button.cc
    widgets/panel.cc
    widgets/panelbox.cc
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
    widgets/collapsingheader.cc
    widgets/selectable.cc
    widgets/riskbar.cc
    widgets/alertbar.cc
    widgets/confirmdialog.cc
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
        widgets/inputtext.cc
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
        widgets/multisplitter.cc
        widgets/dragfloat.cc
        widgets/dragint.cc
        widgets/listbox.cc
        fonts/gradient_text.cc
        widgets/coloredit.cc
        widgets/statuslamp.cc
        widgets/cascadingcombo.cc
        widgets/sliderbar.cc
        widgets/futuresriskbar.cc
        widgets/sparkline.cc
        widgets/gauge.cc
        widgets/segmentedcontrol.cc
        widgets/priceticker.cc
        widgets/pnltext.cc
        widgets/taglist.cc
        widgets/metriccard.cc
        widgets/togglebutton.cc
        widgets/buttongroup.cc
        widgets/connection_status.cc
        widgets/groupedrisktree.cc
        widgets/commandpalette.cc
        widgets/filedialog.cc
    )
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_WIDGETS)
    message(STATUS "Extended widgets enabled")
endif()

# ── Optional modules ──────────────────────────────────────────────────────────
# Each module's implementation lives in src/<module>/, mirroring its public
# header under include/unigui/<module>/.
if(UNIGUI_MODULE_DSL)
    target_sources(unigui PRIVATE dsl/dsl.cc)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_DSL)
    message(STATUS "DSL module enabled")
endif()
if(UNIGUI_MODULE_STYLING)
    target_sources(unigui PRIVATE styling/style_engine.cc)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_STYLING)
    message(STATUS "Styling module enabled")
endif()
if(UNIGUI_MODULE_FONTS)
    target_sources(unigui PRIVATE fonts/font_manager.cc)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_FONTS)
    message(STATUS "Font manager enabled")
endif()
if(UNIGUI_MODULE_EVENTS)
    target_sources(unigui PRIVATE events/eventbus.cc)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_EVENTS)
    message(STATUS "EventBus enabled")
endif()
if(UNIGUI_MODULE_PLUGIN)
    target_sources(unigui PRIVATE plugin/plugin_manager.cc)
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_PLUGIN)
    message(STATUS "Plugin system enabled")
endif()
if(UNIGUI_MODULE_SQLITE)
    target_sources(unigui PRIVATE sqlite/database.cc)
    message(STATUS "SQLite module enabled")
endif()
if(UNIGUI_MODULE_CONFIG)
    target_sources(unigui PRIVATE config/config.cc)
    message(STATUS "Config module enabled")
endif()
if(UNIGUI_MODULE_IPC)
    target_sources(unigui PRIVATE ipc/ipc.cc ipc/shmem.cc)
    message(STATUS "IPC module enabled")
endif()
if(UNIGUI_MODULE_NETWORK)
    target_sources(unigui PRIVATE network/network.cc)
    message(STATUS "Network module enabled")
endif()
if(UNIGUI_MODULE_TRADING)
    # Models (order_book/ohlc_series/quote) are header-only; widgets ship as .cc
    # files. The macro exposes the headers via unigui.h.
    target_sources(unigui PRIVATE
        trading/candlestick_chart.cc
        trading/depth_ladder.cc
        trading/order_ticket.cc
    )
    target_compile_definitions(unigui PUBLIC UNIGUI_HAS_TRADING)
    message(STATUS "Trading toolkit enabled")
endif()

# ── fx effects (core — always compiled) ─────────────────────────────────────
target_sources(unigui PRIVATE     fx/effect_scope.cc fx/animation.cc
    # theme registry
    theme/theme_registry.cc)

# ── Backend sources ───────────────────────────────────────────────────────────
# Shared cross-platform Vulkan renderer (used by both GLFW-Vulkan and SDL3-Vulkan).
if(_UNIGUI_NEED_VULKAN)
    target_sources(unigui PRIVATE backend/vulkan_renderer.cc)
endif()
# Opt-in SDL3 platform backend.
if(UNIGUI_BACKEND_SDL3)
    target_sources(unigui PRIVATE backend/sdl3_platform.cc)
endif()

if(WIN32)
    target_sources(unigui PRIVATE
        backend/metal_renderer.cc
        backend/webgpu_renderer.cc
        backend/emscripten_platform.cc
    )
    # dx11_renderer.cc references d3d11/imgui_impl_dx11 symbols, but those libs are
    # only linked under UNIGUI_BACKEND_DX11. Compiling it unconditionally produced
    # unresolved externals with DX11 OFF. Gate it like dx12_renderer.cc; app.cc and
    # backend_factory.h already guard their DX11 references on UNIGUI_HAS_DX11.
    if(UNIGUI_BACKEND_DX11)
        target_sources(unigui PRIVATE backend/dx11_renderer.cc)
    endif()
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
include(${PROJECT_SOURCE_DIR}/cmake/FontEmbed.cmake)
set(FONT_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated/fonts)
file(MAKE_DIRECTORY ${FONT_GEN_DIR})
embed_font(unigui
    ${PROJECT_SOURCE_DIR}/fonts/JetBrainsMonoNerdFont-Regular.ttf
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
