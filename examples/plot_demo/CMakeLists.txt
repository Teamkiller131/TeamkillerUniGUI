find_package(implot CONFIG REQUIRED)
add_executable(plot_demo main.cc)
target_link_libraries(plot_demo PRIVATE unigui implot::implot)
target_compile_features(plot_demo PRIVATE cxx_std_23)
