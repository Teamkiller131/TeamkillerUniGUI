add_executable(v3_overview "")
target_sources(v3_overview PRIVATE main.cc)
target_link_libraries(v3_overview PRIVATE unigui::unigui)
target_compile_features(v3_overview PRIVATE cxx_std_23)
