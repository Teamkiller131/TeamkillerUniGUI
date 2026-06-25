add_executable(unigui_showcase main.cc)
target_link_libraries(unigui_showcase PRIVATE unigui::unigui)
target_compile_features(unigui_showcase PRIVATE cxx_std_23)
