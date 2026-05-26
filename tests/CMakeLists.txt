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

# v2 widget tests
foreach(_widget checkbox progressbar radiogroup combobox lineedit groupbox tabwidget treeview listview dialog menubar statusbar toolbar table colorpicker filepath dirpath toggleswitch inputint inputfloat splitter separator scrollarea tooltip datepicker image loadingindicator notification hyperlink tag breadcrumb multiline iconbutton space contextmenu shortcut richtext imagebutton markdown virtuallist multicombo propertygrid searchbox toast passwordinput wizard)
    add_executable(test_${_widget} widgets/${_widget}_test.cc)
    target_link_libraries(test_${_widget} PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_${_widget} PRIVATE cxx_std_23)
    gtest_discover_tests(test_${_widget})
endforeach()

# v2.9 core tests
foreach(_core locale settings undo_stack)
    add_executable(test_${_core} core/${_core}_test.cc)
    target_link_libraries(test_${_core} PRIVATE GTest::gtest GTest::gtest_main unigui)
    target_compile_features(test_${_core} PRIVATE cxx_std_23)
    gtest_discover_tests(test_${_core})
endforeach()

# v2.9 benchmark tests
add_executable(test_bench bench/bench_test.cc)
target_link_libraries(test_bench PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_bench PRIVATE cxx_std_23)
gtest_discover_tests(test_bench)

# v1.3 tests — run manually (GL context needed for render/fuzz/font tests)
# add_executable(test_fuzz fuzz/fuzz_test.cc)
# add_executable(test_font theme/font_test.cc)

# v2 plugin tests
add_executable(test_plugin v2/plugin_test.cc)
target_link_libraries(test_plugin PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_plugin PRIVATE cxx_std_23)
gtest_discover_tests(test_plugin)

add_executable(test_dsl v2/dsl_test.cc)
target_link_libraries(test_dsl PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_dsl PRIVATE cxx_std_23)
gtest_discover_tests(test_dsl)

add_executable(test_eventbus v2/eventbus_test.cc)
target_link_libraries(test_eventbus PRIVATE GTest::gtest GTest::gtest_main unigui)
target_compile_features(test_eventbus PRIVATE cxx_std_23)
gtest_discover_tests(test_eventbus)
