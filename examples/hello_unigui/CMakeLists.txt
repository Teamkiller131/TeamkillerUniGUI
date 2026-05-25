# Placeholder: example builds in Task 13
add_executable(hello_unigui "")
target_sources(hello_unigui PRIVATE main.cc)
target_link_libraries(hello_unigui PRIVATE unigui::unigui)
target_compile_features(hello_unigui PRIVATE cxx_std_23)
