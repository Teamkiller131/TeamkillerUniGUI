add_executable(vulkan_triangle main.cc)
target_link_libraries(vulkan_triangle PRIVATE unigui::unigui)
target_compile_features(vulkan_triangle PRIVATE cxx_std_23)
