find_package(implot CONFIG REQUIRED)
add_executable(trading_dashboard main.cc)
target_link_libraries(trading_dashboard PRIVATE unigui implot::implot)
target_compile_features(trading_dashboard PRIVATE cxx_std_23)
