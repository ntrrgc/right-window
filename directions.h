//
// Created by ntrrgc on 25/01/16.
//

#ifndef RIGHT_WINDOW_CPP_DIRECTIONS_H
#define RIGHT_WINDOW_CPP_DIRECTIONS_H

#include "wm_abstract.h"

namespace rw {
    class Direction {
    public:
        virtual int opposite_border(const Window * window) = 0;
        virtual int agreeing_border(const Window * window) = 0;
        virtual bool next_to(int a, int b) = 0;
        virtual bool next_or_equal_to(int a, int b) = 0;
        virtual bool prev_or_equal_to(int a, int b) = 0;
        virtual tuple<int, int> perpendicular_range(const Window * window) = 0;
    };

    class Right : public Direction {
        virtual int opposite_border(const Window * window) {
            return window->left;
        }
        virtual int agreeing_border(const Window * window) {
            return window->right;
        }
        virtual bool next_to(int a, int b) {
            // a and b must be x coordinates
            // a next (right) to b
            return a > b;
        }
        virtual bool next_or_equal_to(int a, int b) {
            return a >= b;
        };
        virtual bool prev_or_equal_to(int a, int b) {
            return a <= b;
        }
        virtual tuple<int, int> perpendicular_range(const Window * window) {
            return make_tuple(window->top, window->bottom);
        };
    };

    class Left : public Direction {
        virtual int opposite_border(const Window * window) {
            return window->right;
        }
        virtual int agreeing_border(const Window * window) {
            return window->left;
        }
        virtual bool next_to(int a, int b) {
            // a and b must be x coordinates
            // a next (left) to b
            return a < b;
        }
        virtual bool next_or_equal_to(int a, int b) {
            return a <= b;
        };
        virtual bool prev_or_equal_to(int a, int b) {
            return a >= b;
        }
        virtual tuple<int, int> perpendicular_range(const Window * window) {
            return make_tuple(window->top, window->bottom);
        };
    };

    class Down : public Direction {
        virtual int opposite_border(const Window * window) {
            return window->top;
        }
        virtual int agreeing_border(const Window * window) {
            return window->bottom;
        }
        virtual bool next_to(int a, int b) {
            // a and b must be y coordinates
            // a next (under) b
            return a > b;
        }
        virtual bool next_or_equal_to(int a, int b) {
            return a >= b;
        };
        virtual bool prev_or_equal_to(int a, int b) {
            return a <= b;
        }
        virtual tuple<int, int> perpendicular_range(const Window * window) {
            return make_tuple(window->left, window->right);
        };
    };

    class Up : public Direction {
        virtual int opposite_border(const Window * window) {
            return window->bottom;
        }
        virtual int agreeing_border(const Window * window) {
            return window->top;
        }
        virtual bool next_to(int a, int b) {
            // a and b must be y coordinates
            // a next (on top of) to b
            return a < b;
        }
        virtual bool next_or_equal_to(int a, int b) {
            return a <= b;
        };
        virtual bool prev_or_equal_to(int a, int b) {
            return a >= b;
        }
        virtual tuple<int, int> perpendicular_range(const Window * window) {
            return make_tuple(window->left, window->right);
        };
    };
}
#endif //RIGHT_WINDOW_CPP_DIRECTIONS_H
