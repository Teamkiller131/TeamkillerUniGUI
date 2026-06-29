# Web (WebAssembly) demo — only meaningful under an Emscripten toolchain.
if(NOT EMSCRIPTEN)
    return()
endif()

add_executable(web_demo main.cc)
target_link_libraries(web_demo PRIVATE unigui::unigui)
target_compile_features(web_demo PRIVATE cxx_std_23)

# Emit a ready-to-open web_demo.html (+ .js + .wasm). ALLOW_MEMORY_GROWTH lets the font
# atlas and ImGui vertex buffers grow past Emscripten's default heap. The
# GLFW/WebGL2/freetype -s settings are applied library-wide by cmake/Emscripten.cmake's
# add_link_options(), so they don't need repeating here.
set_target_properties(web_demo PROPERTIES SUFFIX ".html")
target_link_options(web_demo PRIVATE
    "SHELL:-sALLOW_MEMORY_GROWTH=1"
    "SHELL:-sEXIT_RUNTIME=0")
