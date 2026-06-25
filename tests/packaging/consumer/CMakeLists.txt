# Standalone downstream project that consumes an *installed* UniGUI via
# find_package(). Intentionally NOT part of the main build — it is configured
# separately against an install prefix to validate the exported package
# (see scripts/test_install.* and the install-consume CI job).
cmake_minimum_required(VERSION 3.31)
project(unigui_consumer CXX)

find_package(unigui CONFIG REQUIRED)

add_executable(consumer main.cc)
target_link_libraries(consumer PRIVATE unigui::unigui)
target_compile_features(consumer PRIVATE cxx_std_23)
