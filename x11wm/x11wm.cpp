//
// Created by ntrrgc on 25/01/16.
//

#include "x11wm.h"


#include <iostream>

#undef NDEBUG
#include <cassert>

#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */

namespace rw {

    class NoWindow : public std::exception
    {};

    ::Window get_parent_window(::Display * display, ::Window window) {
        ::Window root;
        ::Window parent;
        ::Window* children;
        unsigned int nchildren;

        int ret = XQueryTree(display, window, &root, &parent, &children, &nchildren);
        assert(ret == true);

        if (children) {
            XFree(children);
        }
        return parent;
    }

    X11WM::X11WM() {
        display = XOpenDisplay(nullptr);
        assert(display != None);

#define LOAD_ATOM(NAME) NAME = XInternAtom(display, #NAME, true); assert(NAME != None);

        LOAD_ATOM(_NET_CLIENT_LIST_STACKING);
        LOAD_ATOM(_NET_WM_DESKTOP);
        LOAD_ATOM(_NET_CURRENT_DESKTOP);
        LOAD_ATOM(_NET_WM_STATE_HIDDEN);
        LOAD_ATOM(_NET_ACTIVE_WINDOW);
        LOAD_ATOM(_NET_WM_STATE);
        LOAD_ATOM(_NET_MOVERESIZE_WINDOW);
        LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_VERT);
        LOAD_ATOM(_NET_WM_STATE_MAXIMIZED_HORZ);

        LOAD_ATOM(_NET_WM_WINDOW_TYPE);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_DESKTOP);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_DOCK);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_TOOLBAR);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_MENU);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_UTILITY);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_SPLASH);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_DIALOG);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_DROPDOWN_MENU);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_POPUP_MENU);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_TOOLTIP);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_NOTIFICATION);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_COMBO);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_DND);
        LOAD_ATOM(_NET_WM_WINDOW_TYPE_NORMAL);
    }

    unique_ptr<WMState> X11WM::get_windows() {
        root_window = XDefaultRootWindow(display);
        assert(root_window != None);

        this->check_virtual_desktops();

        unique_ptr<WMState> state(new WMState);

        Atom propType;
        int propFormat;
        const unsigned long maxSizeWindowList = 256;
        unsigned long window_list_length;
        unsigned long bytes_after_return;
        unsigned char *window_list_raw;
        int ret;

        ret = XGetWindowProperty(display, root_window, _NET_CLIENT_LIST_STACKING,
                                 0, maxSizeWindowList, false,
                                 AnyPropertyType, &propType, &propFormat,
                                 &window_list_length, &bytes_after_return,
                                 &window_list_raw);
        assert(ret == Success);

        const ::Window* window_list = reinterpret_cast<::Window *>(window_list_raw);

        // For each window
        for (unsigned long i = 0; i < window_list_length; ++i) {
            ::Window window_id = window_list[i];

            bool focused;
            rw::Window *rw_window;

            try {
                if (is_minimized(window_id)) {
                    continue;
                }
                if (!is_in_current_desktop(window_id)) {
                    continue;
                }
                if (is_desktop_feature(window_id)) {
                    continue;
                }

                focused = is_focused(window_id);
                rw_window = extract_window_data(window_id);
            } catch (NoWindow&) {
                // Window closed while probing, skip this one.
                continue;
            }

            // The window is visible, add it.
            if (focused) {
                state->focused_window = rw_window;
            } else {
                state->other_windows.push_back(rw_window);
            }
        }

        XFree(window_list_raw);

        return state;
}


    void X11WM::focus(const Window *window) {
        XEvent event;
        long mask = SubstructureRedirectMask | SubstructureNotifyMask;

        event.xclient.type = ClientMessage;
        event.xclient.serial = 0;
        event.xclient.send_event = True;
        event.xclient.message_type = _NET_ACTIVE_WINDOW;
        event.xclient.window = window->window_id;
        event.xclient.format = 32;
        event.xclient.data.l[0] = 0;
        event.xclient.data.l[1] = 0;
        event.xclient.data.l[2] = 0;
        event.xclient.data.l[3] = 0;
        event.xclient.data.l[4] = 0;

        if (!XSendEvent(display, root_window, False, mask, &event)) {
            cerr << "Could not send _NET_ACTIVE_WINDOW event to focus the window."
                 << endl;
            exit(2);
        }
    }

    void X11WM::swap(const Window *a, const Window *b) {
        change_window_state(a, _NET_WM_STATE_REMOVE, _NET_WM_STATE_MAXIMIZED_HORZ);
        change_window_state(a, _NET_WM_STATE_REMOVE, _NET_WM_STATE_MAXIMIZED_VERT);
        change_window_state(b, _NET_WM_STATE_REMOVE, _NET_WM_STATE_MAXIMIZED_HORZ);
        change_window_state(b, _NET_WM_STATE_REMOVE, _NET_WM_STATE_MAXIMIZED_VERT);
        move_resize_window(a, b->left, b->top, b->width(), b->height());
        move_resize_window(b, a->left, a->top, a->width(), a->height());
    }

    Window * X11WM::extract_window_data(::Window x11_window) {
        ::Window parent_window = get_parent_window(display, x11_window);
        ::Window window_to_measure;

        if (parent_window && parent_window != root_window) {
            // Take into account the window border, which hopefully is the
            // parent window
            window_to_measure = parent_window;
            cout << "has parent\n";
        } else {
            // This window has no border. Take the absolute coordinates of the
            // window itself.
            window_to_measure = x11_window;
        }

        int x_in_parent, y_in_parent;
        ::Window root_window_unused;
        unsigned int width, height;
        unsigned int border_width;
        unsigned int depth;
        // Get width, height and x-y coordinates local to the parent.
        int ret = XGetGeometry(display, window_to_measure, &root_window_unused,
                               &x_in_parent, &y_in_parent, &width, &height,
                               &border_width, &depth);
        assert(ret == true);

        // Get coordinates relative to the root windows.
        int x_absolute, y_absolute;
        ::Window child_return;
        ret = XTranslateCoordinates(display, window_to_measure, root_window, 0, 0,
                                    &x_absolute, &y_absolute, &child_return);
        assert(ret == true);

        Window * window = new rw::Window;
        window->window_id = x11_window;
        window->from_xywh(x_absolute, y_absolute, width, height);
        return window;
    }

    X11WM::~X11WM() {
        XCloseDisplay(display);
    }

    void X11WM::check_virtual_desktops() {
        bool exists;
        current_desktop = read_uint32_property(root_window,
                                               _NET_CURRENT_DESKTOP, &exists);
        there_are_virtual_desktops = exists;
    }

    bool X11WM::is_in_current_desktop(::Window window) {
        if (!there_are_virtual_desktops) {
            // If there are no multiple desktop, the window has to be on the
            // only one that exists.
            return true;
        } else {
            bool exists;
            int desktop = read_uint32_property(window, _NET_WM_DESKTOP, &exists);
            return !exists || (desktop == this->current_desktop);
        }
    }

    bool X11WM::is_focused(::Window window) {
        bool exists;
        ::Window activeWindow = read_uint32_property(root_window,
                                                     _NET_ACTIVE_WINDOW, &exists);

        if (!exists) {
            cerr << "This window manager does not support _NET_ACTIVE_WINDOW, "
                            "can't get focused window." << endl;
            exit(2);
        }

        return activeWindow == window;
    }

    uint32_t X11WM::read_uint32_property(unsigned long x11_window,
                                         ::Atom property,
                                         bool *exists)
    {
        Atom actual_type_return;
        int actual_format_return;
        unsigned long nitems_return;
        unsigned long bytes_after_return;
        unsigned char *prop_return;
        int ret;

        ret = XGetWindowProperty(display, x11_window, property,
                                 0, 1, false,
                                 AnyPropertyType,
                                 &actual_type_return, &actual_format_return,
                                 &nitems_return, &bytes_after_return,
                                 &prop_return);
        assert(ret == Success);

        if (actual_type_return == None) {
            *exists = false;
            return 0;
        } else {
            // Virtual desktops are supported
            *exists = true;
            assert(actual_format_return == 32);
            uint32_t value = *reinterpret_cast<uint32_t*>(prop_return);
            XFree(prop_return);
            return value;
        }
    }

    bool X11WM::is_minimized(::Window window) {
        Atom actual_type_return;
        int actual_format_return;
        unsigned long nitems_return;
        unsigned long bytes_after_return;
        unsigned char *prop_return;
        int ret;

        ret = XGetWindowProperty(display, window, _NET_WM_STATE,
                                 0, 128, false,
                                 AnyPropertyType,
                                 &actual_type_return, &actual_format_return,
                                 &nitems_return, &bytes_after_return,
                                 &prop_return);
        assert(ret == Success);

        if (actual_type_return == None) {
            // _NET_WM_STATE is not supported, so this may be a tiling WM
            // Assume not minimized.
            return false;
        }
        assert(actual_format_return == 32);
        Atom * state_hints = reinterpret_cast<Atom*>(prop_return);

        bool found = false;
        for (size_t i = 0; i < nitems_return; ++i) {
            Atom hint = state_hints[i];
            if (hint == _NET_WM_STATE_HIDDEN) {
                found = true;
                break;
            }
        }

        XFree(prop_return);
        return found;
    }

    void X11WM::move_resize_window(const Window * window, int x, int y, unsigned int w, unsigned int h) {
        XEvent event;
        long mask = SubstructureRedirectMask | SubstructureNotifyMask;

        unsigned long gravity_flags = 0;
        // All x, y, w and h are set
        gravity_flags |= (1 << 8);
        gravity_flags |= (1 << 9);
        gravity_flags |= (1 << 10);
        gravity_flags |= (1 << 11);
        gravity_flags |= (1 << 13);

        event.xclient.type = ClientMessage;
        event.xclient.serial = 0;
        event.xclient.send_event = True;
        event.xclient.message_type = _NET_MOVERESIZE_WINDOW;
        event.xclient.window = window->window_id;
        event.xclient.format = 32;
        event.xclient.data.l[0] = (unsigned long) gravity_flags;
        event.xclient.data.l[1] = (unsigned long) x;
        event.xclient.data.l[2] = (unsigned long) y;
        event.xclient.data.l[3] = (unsigned long) w;
        event.xclient.data.l[4] = (unsigned long) h;

        if (!XSendEvent(display, root_window, False, mask, &event)) {
            cerr << "Could not send event to move and resize the window." << endl;
            exit(2);
        }
    }

    void X11WM::change_window_state(const Window *window, ::Atom operation,
                                    ::Atom state)
    {
        XEvent event;
        long mask = SubstructureRedirectMask | SubstructureNotifyMask;

        event.xclient.type = ClientMessage;
        event.xclient.serial = 0;
        event.xclient.send_event = True;
        event.xclient.message_type = _NET_WM_STATE;
        event.xclient.window = window->window_id;
        event.xclient.format = 32;
        event.xclient.data.l[0] = (unsigned long) operation;
        event.xclient.data.l[1] = (unsigned long) state;
        event.xclient.data.l[2] = (unsigned long) 0;
        event.xclient.data.l[3] = (unsigned long) 0;
        event.xclient.data.l[4] = (unsigned long) 0;

        if (!XSendEvent(display, root_window, False, mask, &event)) {
            cerr << "Could not change window state." << endl;
            exit(2);
        }
    }

    bool X11WM::is_desktop_feature(::Window window) {
        Atom actual_type_return;
        int actual_format_return;
        unsigned long nitems_return;
        unsigned long bytes_after_return;
        unsigned char *prop_return;
        int ret;

        ret = XGetWindowProperty(display, window, _NET_WM_WINDOW_TYPE,
                                 0, 128, false,
                                 AnyPropertyType, &actual_type_return,
                                 &actual_format_return,
                                 &nitems_return, &bytes_after_return,
                                 &prop_return);
        assert(ret == Success);

        if (actual_type_return == None) {
            // _NET_WM_WINDOW_TYPE not supported.
            // Assume it is a regular window.
            return false;
        }
        assert(actual_format_return == 32);
        Atom * state_hints = reinterpret_cast<Atom*>(prop_return);

        bool is_desktop_feature = false;
        for (size_t i = 0; i < nitems_return; ++i) {
            Atom hint = state_hints[i];
            if (hint != _NET_WM_WINDOW_TYPE_NORMAL &&
                    hint != _NET_WM_WINDOW_TYPE_DIALOG)
            {
                    is_desktop_feature = true;
                    break;
            }
        }

        XFree(prop_return);
        return is_desktop_feature;
    }
}