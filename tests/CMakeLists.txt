include(GoogleTest)
find_package(GTest CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

add_executable(test_smoke
    test_smoke.cc
)
target_link_libraries(test_smoke PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_smoke PRIVATE cxx_std_23)
gtest_discover_tests(test_smoke)

add_executable(test_context
    core/context_test.cc
)
target_link_libraries(test_context PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_context PRIVATE cxx_std_23)
gtest_discover_tests(test_context)

add_executable(test_error
    core/error_test.cc
)
target_link_libraries(test_error PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_error PRIVATE cxx_std_23)
gtest_discover_tests(test_error)

add_executable(test_id_conflict
    id_conflict_test.cc
)
target_link_libraries(test_id_conflict PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_id_conflict PRIVATE cxx_std_23)
gtest_discover_tests(test_id_conflict)

# C ABI tests — the .c TU is compiled as plain C (proving the header is a real
# C surface) and the .cc TU wraps its checks into gtest cases plus the windowed
# lifecycle run (DX11/WARP, self-gated).
add_executable(test_capi
    capi/capi_test.cc
    capi/capi_c_test.c
)
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/capi/capi_c_test.c
    PROPERTIES LANGUAGE C
)
target_link_libraries(test_capi PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_capi PRIVATE cxx_std_23)
gtest_discover_tests(test_capi)

# Interaction-driven integration tests. With UNIGUI_TEST_ENGINE=ON (and the `testengine`
# vcpkg manifest feature supplying imgui[test-engine]), the Dear ImGui test engine drives
# real clicks/typing through widgets; otherwise the file compiles to a single visible
# skip. The engine's sources are compiled INTO the imgui library by the vcpkg feature,
# so no extra link dependency is needed here.
add_executable(test_integration
    integration_test.cc
)
target_link_libraries(test_integration PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_integration PRIVATE cxx_std_23)
if(UNIGUI_TEST_ENGINE)
    target_compile_definitions(test_integration PRIVATE UNIGUI_TEST_ENGINE=1)
endif()
gtest_discover_tests(test_integration)

# Extended interaction coverage — selection/input + data/composite widgets, the dsl
# application framework, and keyboard-only navigation. These targets exist ONLY when
# the engine is compiled in (unlike test_integration's always-present visible skip).
if(UNIGUI_TEST_ENGINE)
    foreach(_itest widgets framework keyboard presets im capi)
        add_executable(test_interaction_${_itest} interaction/${_itest}_interaction_test.cc)
        target_link_libraries(test_interaction_${_itest} PRIVATE
            GTest::gtest
            GTest::gtest_main
            unigui
        )
        target_include_directories(test_interaction_${_itest} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_compile_features(test_interaction_${_itest} PRIVATE cxx_std_23)
        target_compile_definitions(test_interaction_${_itest} PRIVATE UNIGUI_TEST_ENGINE=1)
        gtest_discover_tests(test_interaction_${_itest})
    endforeach()
    # Trading interaction tests — need the engine AND the trading module together
    # (the windows-msvc-debug-testengine-modules preset provides the combination).
    if(UNIGUI_MODULE_TRADING)
        add_executable(test_interaction_trading interaction/trading_interaction_test.cc)
        target_link_libraries(test_interaction_trading PRIVATE
            GTest::gtest
            GTest::gtest_main
            unigui
        )
        target_include_directories(test_interaction_trading PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        target_compile_features(test_interaction_trading PRIVATE cxx_std_23)
        target_compile_definitions(test_interaction_trading PRIVATE UNIGUI_TEST_ENGINE=1)
        gtest_discover_tests(test_interaction_trading)
    endif()
endif()

add_executable(test_backend_interface
    backend/backend_interface_test.cc
)
target_include_directories(test_backend_interface PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/backend
)
target_link_libraries(test_backend_interface PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_backend_interface PRIVATE cxx_std_23)
gtest_discover_tests(test_backend_interface)

add_executable(test_glfw_gl3_backend
    backend/glfw_gl3_backend_test.cc
)
target_link_libraries(test_glfw_gl3_backend PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_glfw_gl3_backend PRIVATE cxx_std_23)
gtest_discover_tests(test_glfw_gl3_backend)

# WARP (software-rasteriser) headless render smoke for DX11/DX12 — real device
# creation + offscreen clear + GPU readback with no GPU. Only meaningful when a DX
# backend is on (Windows). The DX libs + UNIGUI_HAS_DX* defines arrive transitively
# from the PUBLIC unigui link, so no extra d3d11/d3d12 wiring is needed here; each
# half of the test self-gates on its UNIGUI_HAS_DX* macro.
if(WIN32 AND (UNIGUI_BACKEND_DX11 OR UNIGUI_BACKEND_DX12))
    add_executable(test_dx_warp backend/dx_warp_smoke_test.cc backend/dx_multiviewport_smoke_test.cc)
    target_link_libraries(test_dx_warp PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_dx_warp PRIVATE cxx_std_23)
    gtest_discover_tests(test_dx_warp)
endif()

add_executable(test_theme
    theme/theme_test.cc
)
target_link_libraries(test_theme PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_theme PRIVATE cxx_std_23)
gtest_discover_tests(test_theme)

add_executable(test_style_scope
    theme/style_scope_test.cc
)
target_link_libraries(test_style_scope PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_style_scope PRIVATE cxx_std_23)
gtest_discover_tests(test_style_scope)

add_executable(test_surface_style
    theme/surface_style_test.cc
)
target_link_libraries(test_surface_style PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_surface_style PRIVATE cxx_std_23)
gtest_discover_tests(test_surface_style)

add_executable(test_color_tokens
    theme/color_tokens_test.cc
)
target_link_libraries(test_color_tokens PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_color_tokens PRIVATE cxx_std_23)
gtest_discover_tests(test_color_tokens)

add_executable(test_widget_base
    widgets/widget_base_test.cc
)
target_link_libraries(test_widget_base PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_widget_base PRIVATE cxx_std_23)
gtest_discover_tests(test_widget_base)

add_executable(test_label
    widgets/label_test.cc
)
target_link_libraries(test_label PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_label PRIVATE cxx_std_23)
gtest_discover_tests(test_label)

add_executable(test_button
    widgets/button_test.cc
)
target_link_libraries(test_button PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_button PRIVATE cxx_std_23)
gtest_discover_tests(test_button)

# Fluent With* rollout: compile-time proof (static_asserts over every retained
# widget) that base chainers preserve the derived type, + runtime chain checks.
add_executable(test_fluent_rollout
    widgets/fluent_rollout_test.cc
)
target_link_libraries(test_fluent_rollout PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_fluent_rollout PRIVATE cxx_std_23)
gtest_discover_tests(test_fluent_rollout)

add_executable(test_panel
    widgets/panel_test.cc
)
target_link_libraries(test_panel PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_panel PRIVATE cxx_std_23)
gtest_discover_tests(test_panel)

add_executable(test_form
    widgets/form_test.cc
)
target_link_libraries(test_form PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_form PRIVATE cxx_std_23)
gtest_discover_tests(test_form)

add_executable(test_window
    widgets/window_test.cc
)
target_link_libraries(test_window PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_window PRIVATE cxx_std_23)
gtest_discover_tests(test_window)

add_executable(test_app
    app/app_test.cc
)
target_link_libraries(test_app PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_app PRIVATE cxx_std_23)
gtest_discover_tests(test_app)

# Core widget tests — always compiled
foreach(_widget checkbox progressbar riskbar alertbar radiogroup combobox lineedit groupbox tabwidget separator scrollarea menubar statusbar toolbar dialog collapsingheader selectable panelbox)
    add_executable(test_${_widget} widgets/${_widget}_test.cc)
    target_link_libraries(test_${_widget} PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_${_widget} PRIVATE cxx_std_23)
    gtest_discover_tests(test_${_widget})
endforeach()

# Extended widget tests — only when UNIGUI_MODULE_WIDGETS=ON
if(UNIGUI_MODULE_WIDGETS)
    foreach(_widget treeview listview table slider colorpicker coloredit listbox filepath dirpath toggleswitch inputint inputfloat inputtext splitter multisplitter tooltip datepicker image loadingindicator notification hyperlink tag breadcrumb multiline iconbutton space contextmenu shortcut dragdrop richtext imagebutton markdown virtuallist multicombo propertygrid searchbox toast passwordinput wizard dragfloat dragint statuslamp cascadingcombo sliderbar futuresriskbar sparkline gauge segmentedcontrol priceticker pnltext taglist editabledatagrid metriccard togglebutton buttongroup connection_status groupedrisktree basketticket timeseries_chart commandpalette filedialog databinding flexrow multihandleslider)
        add_executable(test_${_widget} widgets/${_widget}_test.cc)
        target_link_libraries(test_${_widget} PRIVATE GTest::gtest GTest::gtest_main unigui)
        target_compile_features(test_${_widget} PRIVATE cxx_std_23)
        gtest_discover_tests(test_${_widget})
    endforeach()

    # DataTable's numeric-sort helpers (detail::ParseNumericSortKey/CompareSortCells) are
    # inline in the header; this pins the non-throwing-parser behaviour after the std::stod
    # → strutil::TryToDouble fix.
    add_executable(test_datatable_sort widgets/datatable_sort_test.cc)
    target_link_libraries(test_datatable_sort PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_datatable_sort PRIVATE cxx_std_23)
    gtest_discover_tests(test_datatable_sort)
endif()

# Core tests
foreach(_core locale settings strutil undo_stack version format_num observable main_thread widget_pool session_axis decimate layout_store accessibility dpi path_util flex_layout context_registry layout_direction)
    add_executable(test_${_core} core/${_core}_test.cc)
    target_link_libraries(test_${_core} PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_${_core} PRIVATE cxx_std_23)
    gtest_discover_tests(test_${_core})
endforeach()

# Trading toolkit model tests — header-only models, so always built (they only
# need the headers; no UNIGUI_MODULE_TRADING link dependency yet).
foreach(_t order_book ohlc_series quote reactive cell_renderers)
    add_executable(test_${_t} trading/${_t}_test.cc)
    target_link_libraries(test_${_t} PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_${_t} PRIVATE cxx_std_23)
    gtest_discover_tests(test_${_t})
endforeach()

# Trading-model performance benchmarks (header-only models; unique target name
# to avoid colliding with the widget benchmark `test_bench` below).
add_executable(test_trading_bench trading/bench_test.cc)
target_link_libraries(test_trading_bench PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_trading_bench PRIVATE cxx_std_23)
gtest_discover_tests(test_trading_bench)

# Trading widget tests — require the compiled module (.cc lands only when
# UNIGUI_MODULE_TRADING is ON).
if(UNIGUI_MODULE_TRADING)
    foreach(_tw candlestick_chart depth_ladder order_ticket blotters)
        add_executable(test_${_tw} trading/${_tw}_test.cc)
        target_link_libraries(test_${_tw} PRIVATE GTest::gtest GTest::gtest_main unigui)
        target_compile_features(test_${_tw} PRIVATE cxx_std_23)
        gtest_discover_tests(test_${_tw})
    endforeach()
endif()

# UI preset scaffolds (headless: state/fluent/a11y assertions + render smoke).
if(UNIGUI_MODULE_PRESETS AND UNIGUI_MODULE_WIDGETS)
    foreach(_tp app_shell settings_page dashboard master_detail log_console login_page wizard_flow)
        add_executable(test_preset_${_tp} presets/${_tp}_test.cc)
        target_link_libraries(test_preset_${_tp} PRIVATE GTest::gtest GTest::gtest_main unigui)
        target_compile_features(test_preset_${_tp} PRIVATE cxx_std_23)
        gtest_discover_tests(test_preset_${_tp})
    endforeach()
endif()

# v2.9 benchmark tests (requires widgets for VirtualList)
if(UNIGUI_MODULE_WIDGETS)
    add_executable(test_bench bench/bench_test.cc)
    target_link_libraries(test_bench PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_bench PRIVATE cxx_std_23)
    gtest_discover_tests(test_bench)
endif()

# v1.3 tests — run manually (GL context needed for render/fuzz/font tests)
# add_executable(test_fuzz fuzz/fuzz_test.cc)
# add_executable(test_font theme/font_test.cc)

# Parser fuzz tests (no GL context needed)
add_executable(test_parser_fuzz fuzz/parser_fuzz_test.cc)
target_link_libraries(test_parser_fuzz PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_parser_fuzz PRIVATE cxx_std_23)
gtest_discover_tests(test_parser_fuzz)

# Optional-module tests — mirror src/<module>/ and include/unigui/<module>/
if(UNIGUI_MODULE_PLUGIN)
    add_executable(test_plugin plugin/plugin_test.cc)
    target_link_libraries(test_plugin PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_plugin PRIVATE cxx_std_23)
    gtest_discover_tests(test_plugin)
endif()

if(UNIGUI_MODULE_DSL)
    foreach(_d dsl component dsl_app form_component codegen scene_parser)
        add_executable(test_${_d} dsl/${_d}_test.cc)
        target_link_libraries(test_${_d} PRIVATE GTest::gtest GTest::gtest_main unigui)
        target_compile_features(test_${_d} PRIVATE cxx_std_23)
        gtest_discover_tests(test_${_d})
    endforeach()
endif()

if(UNIGUI_MODULE_EVENTS)
    add_executable(test_eventbus events/eventbus_test.cc)
    target_link_libraries(test_eventbus PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_eventbus PRIVATE cxx_std_23)
    gtest_discover_tests(test_eventbus)
endif()

if(UNIGUI_MODULE_STYLING)
    add_executable(test_style styling/style_test.cc)
    target_link_libraries(test_style PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_style PRIVATE cxx_std_23)
    gtest_discover_tests(test_style)

    # CSS parser fuzz target (no GL context needed)
    add_executable(test_css_fuzz fuzz/css_fuzz_test.cc)
    target_link_libraries(test_css_fuzz PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_css_fuzz PRIVATE cxx_std_23)
    gtest_discover_tests(test_css_fuzz)
endif()

if(UNIGUI_MODULE_FONTS)
    add_executable(test_fontmgr fonts/font_test.cc)
    target_link_libraries(test_fontmgr PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_fontmgr PRIVATE cxx_std_23)
    gtest_discover_tests(test_fontmgr)
endif()

if(UNIGUI_MODULE_CONFIG)
    add_executable(test_config config/config_test.cc)
    target_link_libraries(test_config PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_config PRIVATE cxx_std_23)
    gtest_discover_tests(test_config)

    # Config parser fuzz target (TOML/JSON/INI, no GL context needed)
    add_executable(test_config_fuzz fuzz/config_fuzz_test.cc)
    target_link_libraries(test_config_fuzz PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_config_fuzz PRIVATE cxx_std_23)
    gtest_discover_tests(test_config_fuzz)
endif()

if(UNIGUI_MODULE_SQLITE)
    add_executable(test_database sqlite/database_test.cc)
    target_link_libraries(test_database PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_database PRIVATE cxx_std_23)
    gtest_discover_tests(test_database)
endif()

if(UNIGUI_MODULE_IPC)
    add_executable(test_ipc ipc/ipc_test.cc)
    target_link_libraries(test_ipc PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_ipc PRIVATE cxx_std_23)
    gtest_discover_tests(test_ipc)
    add_executable(test_shmem ipc/shmem_test.cc)
    target_link_libraries(test_shmem PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_shmem PRIVATE cxx_std_23)
    gtest_discover_tests(test_shmem)
endif()

if(UNIGUI_MODULE_NETWORK)
    add_executable(test_websocket network/websocket_test.cc)
    target_link_libraries(test_websocket PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_websocket PRIVATE cxx_std_23)
    gtest_discover_tests(test_websocket)

    add_executable(test_url_parse network/url_parse_test.cc)
    target_link_libraries(test_url_parse PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_url_parse PRIVATE cxx_std_23)
    gtest_discover_tests(test_url_parse)

    # Live loopback transport tests: httplib + ixwebsocket propagate transitively from
    # unigui (linked PUBLIC), so no extra find_package is needed here.
    add_executable(test_http_loopback network/http_loopback_test.cc)
    target_link_libraries(test_http_loopback PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_http_loopback PRIVATE cxx_std_23)
    gtest_discover_tests(test_http_loopback)

    add_executable(test_ws_loopback network/ws_loopback_test.cc)
    target_link_libraries(test_ws_loopback PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_ws_loopback PRIVATE cxx_std_23)
    gtest_discover_tests(test_ws_loopback)
endif()

# v3.0 fx tests
add_executable(test_easing fx/easing_test.cc)
target_link_libraries(test_easing PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_easing PRIVATE cxx_std_23)
gtest_discover_tests(test_easing)

add_executable(test_animation fx/animation_test.cc)
target_link_libraries(test_animation PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_animation PRIVATE cxx_std_23)
gtest_discover_tests(test_animation)

# v3.0 effect benchmarks
add_executable(test_effect_bench fx/bench_test.cc)
target_link_libraries(test_effect_bench PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_effect_bench PRIVATE cxx_std_23)
gtest_discover_tests(test_effect_bench)

# Step 4: theme-driven elevation tokens (pure — no GL context needed)
add_executable(test_elevation fx/elevation_test.cc)
target_link_libraries(test_elevation PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_elevation PRIVATE cxx_std_23)
gtest_discover_tests(test_elevation)

# Usability layer: immediate-mode API (unigui::im)
add_executable(test_im im/im_test.cc)
target_link_libraries(test_im PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_im PRIVATE cxx_std_23)
gtest_discover_tests(test_im)

# RTL mirroring geometry (headless — layout math only, no render backend)
add_executable(test_im_rtl im/im_rtl_test.cc)
target_link_libraries(test_im_rtl PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_im_rtl PRIVATE cxx_std_23)
gtest_discover_tests(test_im_rtl)

# Usability layer: RAII scope guards (unigui::WindowScope, ...)
add_executable(test_scope core/scope_test.cc)
target_link_libraries(test_scope PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_scope PRIVATE cxx_std_23)
gtest_discover_tests(test_scope)

# Usability layer: Make<T>/MakeNamed<T> factory helpers
add_executable(test_make core/make_test.cc)
target_link_libraries(test_make PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_make PRIVATE cxx_std_23)
gtest_discover_tests(test_make)

# Usability layer: CRTP fluent Button API
add_executable(test_button_fluent widgets/button_fluent_test.cc)
target_link_libraries(test_button_fluent PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_button_fluent PRIVATE cxx_std_23)
gtest_discover_tests(test_button_fluent)

# v3.2.7 sticky header tests (manual — needs render backend)
# add_executable(test_datatable_sticky widgets/datatable_sticky_test.cc)

# v3.2.7 font reload tests
add_executable(test_font_reload theme/font_reload_test.cc)
target_link_libraries(test_font_reload PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_font_reload PRIVATE cxx_std_23)
gtest_discover_tests(test_font_reload)

# v3.0 effect scope tests — manual-only (needs GL context)
# add_executable(test_effect_scope fx/effect_scope_test.cc)

# ── Warnings ─────────────────────────────────────────────────────────────────
# Apply UniGUI's base warning set (and opt-in -Werror) to every test target
# defined above, so the test tree is held to the same standard as the library.
get_property(_unigui_test_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
foreach(_tt ${_unigui_test_targets})
    get_target_property(_tt_type ${_tt} TYPE)
    if(_tt_type STREQUAL "EXECUTABLE")
        unigui_set_warnings(${_tt})
    endif()
endforeach()
