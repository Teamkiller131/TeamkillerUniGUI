add_executable(designer "")
target_sources(designer PRIVATE main.cc)
target_link_libraries(designer PRIVATE unigui::unigui)
target_compile_features(designer PRIVATE cxx_std_23)
