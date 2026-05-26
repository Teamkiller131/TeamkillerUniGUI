add_library(example_plugin SHARED plugin.cc)
target_link_libraries(example_plugin PRIVATE unigui)
target_compile_features(example_plugin PRIVATE cxx_std_23)
set_target_properties(example_plugin PROPERTIES PREFIX "")
