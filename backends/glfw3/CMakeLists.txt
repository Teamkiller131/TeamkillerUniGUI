add_library(unigui-backend-glfw3 "")
add_library(unigui::backend-glfw3 ALIAS unigui-backend-glfw3)
set_target_properties(unigui-backend-glfw3 PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_link_libraries(unigui-backend-glfw3 PUBLIC unigui::core)
target_include_directories(unigui-backend-glfw3 PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_sources(unigui-backend-glfw3 PRIVATE
    ${CMAKE_SOURCE_DIR}/src/backend/glfw_platform.cc
    ${CMAKE_SOURCE_DIR}/src/backend/opengl3_renderer.cc
)
