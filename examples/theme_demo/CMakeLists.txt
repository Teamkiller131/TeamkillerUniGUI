add_executable(theme_demo "")
target_sources(theme_demo PRIVATE main.cc)
target_link_libraries(theme_demo PRIVATE unigui::unigui)
target_compile_features(theme_demo PRIVATE cxx_std_23)
