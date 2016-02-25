cmake_minimum_required(VERSION 2.8)
project(right_window_cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -Wfatal-errors -Werror -Wno-error=unused-variable -Wno-error=unused-but-set-variable")

include_directories("vendor")

find_package(X11 REQUIRED)


set(SOURCE_FILES
        main.cpp
        wm_abstract.h
        directions.h
        available_wm.h
        available_wm.cpp
        rw_util.h
        find_next_window.h
        find_next_window.cpp)

add_executable(right-window-bspwm ${SOURCE_FILES}
        subprocess.cpp
        bspwm/bspwm.h
        bspwm/bspwm.cpp window_clipping.h)
set_target_properties(right-window-bspwm PROPERTIES COMPILE_FLAGS "-DUSE_BSPWM")

add_executable(right-window-x11 ${SOURCE_FILES}
        x11wm/x11wm.h
        x11wm/x11wm.cpp)
set_target_properties(right-window-x11 PROPERTIES COMPILE_FLAGS "-DUSE_X11WM")
include_directories(right-window-x11 PUBLIC ${X11_INCLUDE_DIR})
target_link_libraries(right-window-x11 ${X11_LIBRARIES})
