add_executable(framework_demo main.cc)
target_link_libraries(framework_demo PRIVATE unigui::unigui)
target_compile_features(framework_demo PRIVATE cxx_std_23)
