# preset_demo — a complete app assembled only from the unigui::presets scaffolds.
add_executable(preset_demo "")
target_sources(preset_demo PRIVATE main.cc)
target_link_libraries(preset_demo PRIVATE unigui::unigui)
target_compile_features(preset_demo PRIVATE cxx_std_23)
