add_library(unigui-widgets "")
add_library(unigui::widgets ALIAS unigui-widgets)
set_target_properties(unigui-widgets PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_link_libraries(unigui-widgets PUBLIC unigui::core)
target_include_directories(unigui-widgets PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_sources(unigui-widgets PRIVATE
    ${CMAKE_SOURCE_DIR}/src/widgets/treeview.cc ${CMAKE_SOURCE_DIR}/src/widgets/listview.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/table.cc ${CMAKE_SOURCE_DIR}/src/widgets/colorpicker.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/filepath.cc ${CMAKE_SOURCE_DIR}/src/widgets/dirpath.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/toggleswitch.cc ${CMAKE_SOURCE_DIR}/src/widgets/inputint.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/inputfloat.cc ${CMAKE_SOURCE_DIR}/src/widgets/splitter.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/datepicker.cc ${CMAKE_SOURCE_DIR}/src/widgets/image.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/loadingindicator.cc ${CMAKE_SOURCE_DIR}/src/widgets/notification.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/hyperlink.cc ${CMAKE_SOURCE_DIR}/src/widgets/tag.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/breadcrumb.cc ${CMAKE_SOURCE_DIR}/src/widgets/multiline.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/iconbutton.cc ${CMAKE_SOURCE_DIR}/src/widgets/space.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/contextmenu.cc ${CMAKE_SOURCE_DIR}/src/widgets/richtext.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/imagebutton.cc ${CMAKE_SOURCE_DIR}/src/widgets/markdown.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/virtuallist.cc ${CMAKE_SOURCE_DIR}/src/widgets/multicombo.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/propertygrid.cc ${CMAKE_SOURCE_DIR}/src/widgets/searchbox.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/toast.cc ${CMAKE_SOURCE_DIR}/src/widgets/passwordinput.cc
    ${CMAKE_SOURCE_DIR}/src/widgets/wizard.cc ${CMAKE_SOURCE_DIR}/src/widgets/trayicon.cc
)
