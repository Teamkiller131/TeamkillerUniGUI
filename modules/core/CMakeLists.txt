add_library(unigui-core "")
add_library(unigui::core ALIAS unigui-core)
set_target_properties(unigui-core PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
find_package(imgui CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(unigui-core PUBLIC imgui::imgui glfw glad::glad Freetype::Freetype spdlog::spdlog implot::implot)
target_include_directories(unigui-core PUBLIC ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})
target_sources(unigui-core PRIVATE
    ../../src/unigui.cc
    ../../src/core/context.cc ../../src/core/error.cc
    ../../src/core/locale.cc ../../src/core/settings.cc ../../src/core/log.cc
    ../../src/theme/theme.cc ../../src/theme/style_scope.cc
    ../../src/widgets/widget_base.cc ../../src/widgets/label.cc
    ../../src/widgets/button.cc ../../src/widgets/panel.cc
    ../../src/widgets/window.cc ../../src/widgets/form.cc
    ../../src/widgets/checkbox.cc ../../src/widgets/progressbar.cc
    ../../src/widgets/radiogroup.cc ../../src/widgets/combobox.cc
    ../../src/widgets/lineedit.cc ../../src/widgets/groupbox.cc
    ../../src/widgets/tabwidget.cc ../../src/widgets/separator.cc
    ../../src/widgets/menubar.cc ../../src/widgets/statusbar.cc
    ../../src/widgets/toolbar.cc ../../src/widgets/tooltip.cc
    ../../src/widgets/scrollarea.cc ../../src/widgets/dialog.cc
    ../../src/app/app.cc ../../src/backend/backend_registry.cc
)
if(WIN32) 
    target_compile_definitions(unigui-core PUBLIC UNIGUI_HAS_DX11 UNIGUI_HAS_DX12)
endif()
