add_executable(form_demo main.cc)
target_link_libraries(form_demo PRIVATE unigui::unigui)
target_compile_features(form_demo PRIVATE cxx_std_23)
