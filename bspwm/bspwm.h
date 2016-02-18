//
// Created by ntrrgc on 25/01/16.
//

#ifndef RIGHT_WINDOW_CPP_BSPWM_H
#define RIGHT_WINDOW_CPP_BSPWM_H

#include "../wm_abstract.h"
#include <memory>

namespace rw {

    class BSPWM : public WindowManager {
    public:
        unique_ptr<WMState> get_windows();

        void focus(const Window *window);

        void swap(const Window *a, const Window *b);
    };

}

#endif //RIGHT_WINDOW_CPP_BSPWM_H
