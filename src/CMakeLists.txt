set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

add_definitions(-DXPLM301 -DXPLM300 -DXPLM210 -DXPLM200)
add_definitions(-DMOVEVR_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("lib/XSDK/CHeaders")
include_directories("lib/imgui")

# X-Plane plugin
add_library(movevr_plugin SHARED
    ${CMAKE_CURRENT_LIST_DIR}/Plugin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Logger.cpp
)

include("${CMAKE_CURRENT_LIST_DIR}/MoveVR/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/windows/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/xplane/CMakeLists.txt")

if(WIN32)
    add_definitions(-DIBM)
    target_link_libraries(movevr_plugin
        -static
        -static-libgcc
        -static-libstdc++
        movevr_imgui
        "${PROJECT_SOURCE_DIR}/lib/XSDK/Libraries/Win/XPLM_64.lib"
        gdiplus
        OpenGL32
    )
endif(WIN32)

set_target_properties(movevr_plugin PROPERTIES PREFIX "")
set_target_properties(movevr_plugin PROPERTIES OUTPUT_NAME "MoveVR")
set_target_properties(movevr_plugin PROPERTIES SUFFIX ".xpl")

if(WIN32)
install(TARGETS movevr_plugin
    RUNTIME DESTINATION  "/Users/rme/Desktop/X-Plane 11/Resources/plugins/MoveVR/win_x64/"
)
endif(WIN32)
