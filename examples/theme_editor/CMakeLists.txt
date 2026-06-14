add_executable(theme_editor "")
target_sources(theme_editor PRIVATE main.cc)
target_link_libraries(theme_editor PRIVATE unigui::unigui)
target_compile_features(theme_editor PRIVATE cxx_std_23)
