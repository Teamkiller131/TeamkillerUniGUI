add_executable(widget_gallery main.cc)
target_link_libraries(widget_gallery PRIVATE unigui::unigui)
target_compile_features(widget_gallery PRIVATE cxx_std_23)
