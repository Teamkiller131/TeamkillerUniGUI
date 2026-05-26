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
foreach(_widget checkbox progressbar radiogroup combobox lineedit groupbox tabwidget separator scrollarea menubar statusbar toolbar dialog)
    add_executable(test_${_widget} widgets/${_widget}_test.cc)
    target_link_libraries(test_${_widget} PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_${_widget} PRIVATE cxx_std_23)
    gtest_discover_tests(test_${_widget})
endforeach()

# Extended widget tests — only when UNIGUI_MODULE_WIDGETS=ON
if(UNIGUI_MODULE_WIDGETS)
    foreach(_widget treeview listview table colorpicker filepath dirpath toggleswitch inputint inputfloat splitter tooltip datepicker image loadingindicator notification hyperlink tag breadcrumb multiline iconbutton space contextmenu shortcut richtext imagebutton markdown virtuallist multicombo propertygrid searchbox toast passwordinput wizard)
        add_executable(test_${_widget} widgets/${_widget}_test.cc)
        target_link_libraries(test_${_widget} PRIVATE GTest::gtest GTest::gtest_main unigui)
        target_compile_features(test_${_widget} PRIVATE cxx_std_23)
        gtest_discover_tests(test_${_widget})
    endforeach()
endif()

# v2.9 core tests
foreach(_core locale settings undo_stack)
    add_executable(test_${_core} core/${_core}_test.cc)
    target_link_libraries(test_${_core} PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_${_core} PRIVATE cxx_std_23)
    gtest_discover_tests(test_${_core})
endforeach()

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

# v2 plugin tests
if(UNIGUI_MODULE_PLUGIN)
    add_executable(test_plugin v2/plugin_test.cc)
    target_link_libraries(test_plugin PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_plugin PRIVATE cxx_std_23)
    gtest_discover_tests(test_plugin)
endif()

if(UNIGUI_MODULE_DSL)
    add_executable(test_dsl v2/dsl_test.cc)
    target_link_libraries(test_dsl PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_dsl PRIVATE cxx_std_23)
    gtest_discover_tests(test_dsl)
endif()

if(UNIGUI_MODULE_EVENTS)
    add_executable(test_eventbus v2/eventbus_test.cc)
    target_link_libraries(test_eventbus PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_eventbus PRIVATE cxx_std_23)
    gtest_discover_tests(test_eventbus)
endif()

if(UNIGUI_MODULE_STYLING)
    add_executable(test_style v2/style_test.cc)
    target_link_libraries(test_style PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_style PRIVATE cxx_std_23)
    gtest_discover_tests(test_style)
endif()

if(UNIGUI_MODULE_FONTS)
    add_executable(test_fontmgr v2/font_test.cc)
    target_link_libraries(test_fontmgr PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_fontmgr PRIVATE cxx_std_23)
    gtest_discover_tests(test_fontmgr)
endif()

if(UNIGUI_MODULE_CONFIG)
    add_executable(test_config v2/config_test.cc)
    target_link_libraries(test_config PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_config PRIVATE cxx_std_23)
    gtest_discover_tests(test_config)
endif()

if(UNIGUI_MODULE_SQLITE)
    add_executable(test_database v2/database_test.cc)
    target_link_libraries(test_database PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_database PRIVATE cxx_std_23)
    gtest_discover_tests(test_database)
endif()

if(UNIGUI_MODULE_IPC)
    add_executable(test_ipc v2/ipc_test.cc)
    target_link_libraries(test_ipc PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_ipc PRIVATE cxx_std_23)
    gtest_discover_tests(test_ipc)
add_executable(test_shmem v2/shmem_test.cc)
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

# v3.0 effect scope tests — manual-only (needs GL context)
# add_executable(test_effect_scope fx/effect_scope_test.cc)
