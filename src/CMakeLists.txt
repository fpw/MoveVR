set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -fPIC")

add_definitions(-DIBM=1 -DXPLM301=1 -DXPLM300=1 -DXPLM210=1 -DXPLM200=1)

add_subdirectory(imgui)
add_subdirectory(windows)
add_subdirectory(xplane)
add_subdirectory(MoveVR)

# X-Plane plugin
add_library(movevr_plugin SHARED Plugin.cpp Logger.cpp)
target_link_libraries(movevr_plugin
    -static
    -static-libgcc
    -static-libstdc++
    movevr_main
    movevr_windows
    movevr_imgui
    movevr_xplane
    gdiplus
    XPLM_64
    OpenGL32
)

set_target_properties(movevr_plugin PROPERTIES PREFIX "")
set_target_properties(movevr_plugin PROPERTIES OUTPUT_NAME "MoveVR")
set_target_properties(movevr_plugin PROPERTIES SUFFIX ".xpl")

if(WIN32)
install(TARGETS movevr_plugin
    RUNTIME DESTINATION  "/Users/rme/Desktop/X-Plane 11/Resources/plugins/MoveVR/win_x64/"
)
endif(WIN32)
