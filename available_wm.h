//
// Created by ntrrgc on 25/01/16.
//

#ifndef RIGHT_WINDOW_CPP_AVAILABLE_WM_H
#define RIGHT_WINDOW_CPP_AVAILABLE_WM_H

#include "wm_abstract.h"

namespace rw {
    unique_ptr<WindowManager> get_window_manager();
}

#endif //RIGHT_WINDOW_CPP_AVAILABLE_WM_H
