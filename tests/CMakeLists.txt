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

add_executable(test_integration
    integration_test.cc
)
target_link_libraries(test_integration PRIVATE
    GTest::gtest
    GTest::gtest_main
    unigui
)
target_compile_features(test_integration PRIVATE cxx_std_23)
gtest_discover_tests(test_integration)

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
    foreach(_widget treeview listview table slider colorpicker coloredit listbox filepath dirpath toggleswitch inputint inputfloat inputtext splitter multisplitter tooltip datepicker image loadingindicator notification hyperlink tag breadcrumb multiline iconbutton space contextmenu shortcut dragdrop richtext imagebutton markdown virtuallist multicombo propertygrid searchbox toast passwordinput wizard dragfloat dragint statuslamp cascadingcombo sliderbar futuresriskbar sparkline gauge segmentedcontrol priceticker pnltext taglist editabledatagrid metriccard togglebutton buttongroup connection_status groupedrisktree basketticket timeseries_chart commandpalette filedialog databinding flexrow)
        add_executable(test_${_widget} widgets/${_widget}_test.cc)
        target_link_libraries(test_${_widget} PRIVATE GTest::gtest GTest::gtest_main unigui)
        target_compile_features(test_${_widget} PRIVATE cxx_std_23)
        gtest_discover_tests(test_${_widget})
    endforeach()
endif()

# Core tests
foreach(_core locale settings strutil undo_stack version format_num observable main_thread widget_pool session_axis decimate layout_store accessibility dpi path_util flex_layout)
    add_executable(test_${_core} core/${_core}_test.cc)
    target_link_libraries(test_${_core} PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_${_core} PRIVATE cxx_std_23)
    gtest_discover_tests(test_${_core})
endforeach()

# Trading toolkit model tests — header-only models, so always built (they only
# need the headers; no UNIGUI_MODULE_TRADING link dependency yet).
foreach(_t order_book ohlc_series quote reactive)
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
    foreach(_d dsl component)
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
