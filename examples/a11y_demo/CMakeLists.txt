add_executable(a11y_demo main.cc)
target_link_libraries(a11y_demo PRIVATE unigui::unigui)
target_compile_features(a11y_demo PRIVATE cxx_std_23)
