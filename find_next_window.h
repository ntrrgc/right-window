//
// Created by ntrrgc on 26/01/16.
//

#ifndef RIGHT_WINDOW_CPP_FIND_NEXT_WINDOW_H
#define RIGHT_WINDOW_CPP_FIND_NEXT_WINDOW_H


namespace rw {
    class Window;
    class WMState;
    class Direction;

    Window * find_next_window(WMState * wm, Direction * direction, bool debug);
}

#endif //RIGHT_WINDOW_CPP_FIND_NEXT_WINDOW_H
