//
// Created by ntrrgc on 25/01/16.
//

#ifndef RIGHT_WINDOW_CPP_WM_ABSTRACT_H
#define RIGHT_WINDOW_CPP_WM_ABSTRACT_H

#include <tuple>
#include <vector>
#include <memory>
#include <iomanip>

namespace rw {

    using namespace std;

    class WMState;
    class Window;

    class WindowManager {
    public:
        virtual ~WindowManager() {}

        // Returns a WMState object containing:
        //
        // * The currently focused window (or nullptr if not any) in
        //   WMState::focused_window.
        //
        // * A vector of the other currently open visible windows, ordered by
        //   stack position. The topmost window must appear as the last element
        //   in the vector. The focused window must not be included.
        //   Minimized windows and desktop features such as taskbars
        //   which are not to be focused should not appear either.
        virtual unique_ptr<WMState> get_windows() = 0;

        // Focuses a window.
        virtual void focus(const Window *window) = 0;

        // Swaps position and size of two windows.
        virtual void swap(const Window *a, const Window *b) = 0;
    };

    class Window {
    public:
        int window_id; // An id used to manipulate this window (e.g. to request
                       // the WM to focus it)
        int top; // Y coordinate of the top border of the window.
        int bottom; // X coordinate of the right border of the window.
        int left; // Y coordinate of the bottom border of the window.
        int right; // X coordinate of the left border of the window.

        // The top left corner of the screen is at x = 0, y = 0 in this
        // convention.

        void from_xywh(int x, int y, unsigned int w, unsigned int h) {
            left = x;
            right = x + w;
            top = y;
            bottom = y + h;
        }

        tuple<int, int> center() const {
            return make_tuple((left + right) / 2,
                              (top + bottom) / 2);
        };

        unsigned int width() const {
            return right - left;
        }
        unsigned int height() const {
            return bottom - top;
        }

    };

    template<typename T>
    T &operator<<(T &stream, const Window &w) {
        return stream << "Window(0x" << hex << w.window_id << dec << " " <<
               "top=" << w.top << ", " <<
               "bottom=" << w.bottom << ", " <<
               "left=" << w.left << ", " <<
               "right=" << w.right << ")";
    }

    template<typename T>
    T &operator<<(T &stream, const Window *w) {
        if (w) {
            return stream << (*w);
        } else {
            return stream << "(null)";
        }
    }

    template<typename T>
    T &operator<<(T &stream, const vector<Window *> v) {
        stream << "[";
        auto i = v.begin();
        if (i != v.end()) {
            stream << *i;
            i++;
        }
        while (i != v.end()) {
            stream << ",\n " << *i;
            i++;
        }
        stream << "]";
        return stream;
    }

    class WMState {
    public:
        Window *focused_window;
        vector<Window *> other_windows; // most recent window last

        WMState() : focused_window(nullptr) {}

        virtual ~WMState() {
            if (focused_window) {
                delete focused_window;
            }
            for (Window *window : other_windows) {
                delete window;
            }
        }
    };

}

#endif //RIGHT_WINDOW_CPP_WM_ABSTRACT_H
