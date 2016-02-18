//
// Created by ntrrgc on 25/01/16.
//

#ifndef RIGHT_WINDOW_CPP_X11WM_H
#define RIGHT_WINDOW_CPP_X11WM_H

#include "../wm_abstract.h"
#include <X11/Xlib.h>
#include <boost/concept_check.hpp>

namespace rw {

    using namespace std;

    class X11WM : public WindowManager {
    public:
        X11WM();
        ~X11WM();

        unique_ptr<WMState> get_windows();

        void focus(const Window *window);

        void swap(const Window *a, const Window *b);

    private:
        ::Display * display;
        ::Window root_window;
        bool there_are_virtual_desktops;
        int current_desktop;
        void check_virtual_desktops();

        uint32_t read_uint32_property(::Window x11_window, ::Atom property, bool *exists);

        ::Atom _NET_CLIENT_LIST_STACKING;
        ::Atom _NET_WM_DESKTOP;
        ::Atom _NET_CURRENT_DESKTOP;
        ::Atom _NET_WM_STATE_HIDDEN;
        ::Atom _NET_ACTIVE_WINDOW;
        ::Atom _NET_WM_STATE;
        ::Atom _NET_MOVERESIZE_WINDOW;
        ::Atom _NET_WM_STATE_MAXIMIZED_VERT;
        ::Atom _NET_WM_STATE_MAXIMIZED_HORZ;

        ::Atom _NET_WM_WINDOW_TYPE;
        ::Atom _NET_WM_WINDOW_TYPE_DESKTOP;
        ::Atom _NET_WM_WINDOW_TYPE_DOCK;
        ::Atom _NET_WM_WINDOW_TYPE_TOOLBAR;
        ::Atom _NET_WM_WINDOW_TYPE_MENU;
        ::Atom _NET_WM_WINDOW_TYPE_UTILITY;
        ::Atom _NET_WM_WINDOW_TYPE_SPLASH;
        ::Atom _NET_WM_WINDOW_TYPE_DIALOG;
        ::Atom _NET_WM_WINDOW_TYPE_DROPDOWN_MENU;
        ::Atom _NET_WM_WINDOW_TYPE_POPUP_MENU;
        ::Atom _NET_WM_WINDOW_TYPE_TOOLTIP;
        ::Atom _NET_WM_WINDOW_TYPE_NOTIFICATION;
        ::Atom _NET_WM_WINDOW_TYPE_COMBO;
        ::Atom _NET_WM_WINDOW_TYPE_DND;
        ::Atom _NET_WM_WINDOW_TYPE_NORMAL;

        bool is_focused(::Window window);
        bool is_in_current_desktop(::Window window);
        bool is_minimized(::Window window);
        bool is_desktop_feature(::Window window);
        Window * extract_window_data(::Window x11_window);

        void move_resize_window(const Window * window, int x, int y, unsigned int w, unsigned int h);
        void change_window_state(const Window * window, ::Atom operation, ::Atom state);
    };

}

#endif //RIGHT_WINDOW_CPP_X11WM_H
