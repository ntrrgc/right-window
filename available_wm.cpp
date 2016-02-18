//
// Created by ntrrgc on 26/01/16.
//
#include "available_wm.h"

#ifdef USE_BSPWM
#include "bspwm/bspwm.h"
#define COMPILED_WM BSPWM
#elif defined(USE_X11WM)
#include "x11wm/x11wm.h"
#define COMPILED_WM X11WM
#endif

namespace rw {
    unique_ptr<WindowManager> get_window_manager() {
        return unique_ptr<WindowManager>(new COMPILED_WM());
    }
}