# imgui
add_definitions(-DIMGUI_USER_CONFIG="../imconfig.h")
add_library(movevr_imgui
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/imgui.cpp
    lib/imgui/misc/cpp/imgui_stdlib.cpp
)

if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8)
    target_compile_options(movevr_imgui PRIVATE "-Wno-stringop-truncation")
endif()
