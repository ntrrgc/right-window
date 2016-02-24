right-window
============

This is a small utility to move the focus between windows in a desktop environment following a cardinal direction. For instance:
 
    right-window -f right
    
will focus the window next to the right. Available directions are `left`, `right`, `up` and `down`. 

The `-f` parameter tells the `right-window` command to focus the found window . Alternatively, `-s` *switches* the current window with the found window, swapping their position and size.

You can also use `-g` to print the system identifier of the matched window. This is useful if you want to use `right-window` as a starting point for creating other utilities.
 
The command is intended to be bound as a series of hotkeys, e.g:
 
    Win+H: right-window -f left
    Win+L: right-window -f right
    Win+K: right-window -f up
    Win+J: right-window -f down
    
    Win+Shift+H: right-window -s left
    Win+Shift+L: right-window -s right
    Win+Shift+K: right-window -s up
    Win+Shift+J: right-window -s down

Multi-monitor is supported.

Desktop support
---------------

This utility was written with the [bspwm](https://github.com/baskerville/bspwm) window manager in mind, as an alternative to similar commands like `bspc node --focus east` but having more intuitive multi monitor support (see [bspwm issue #380](https://github.com/baskerville/bspwm/issues/380)).

There are currently two different binary builds of `right-window`:

* `right-window-bspwm`: Uses `bspc` command as a subprocess to query and manipulate the window manager. This is the recommended way to use the utility in bspwm. Building it does not require to have bspwm installed.

* `right-window-x11`: Uses Xlib as an alternative for other X11-based window managers. It uses common `_NET_WM` extensions to interact with the window manager. 

  The focus command `-f` should work fine on most window managers. Unfortunately, swapping windows with `-s` does not work completely well on many window managers for a lack of a consistent way of querying and setting window position in X11. 
  
  Common issues are negative positions not being supported (see `_NET_MOVERESIZE_WINDOW` specification), window borders not accounted and spurious offsets. Help in this area is appreciated.

Adding support for other window managers is possible implementing the interface `rw::WindowManager` from `wm_abstract.h`. All that is required are three methods for querying the list of currently active windows, focusing them and swapping their position and size. See the comments in the source code for more details. Additionally a compiler flag and an `#include` directive pointing to the new window manager class must be added to `available_wm.cpp` and `CMakeLists.txt`.

Building
--------

You can use the typical building procedure for a CMake project:

    mkdir build && cd build
    cmake <path to project>
    make

Speed
-----

This tool is meant to be executed each time a movement hotkey is pressed, so the program execution time matters. Initially it was written in Python, but later the tool was rewritten in C++ to drastically reduce the process time from 60-80 milliseconds Python requires just to load the interpreter to only 2 milliseconds which is way under a frame, as long as the executable is cached in memory.

The measurements were made on a Linux system.

Window search algorithm
-----------------------

The problem of finding the next window to the right (or any other direction) is actually harder than it looks since there are many ambiguous cases.

This is the algorithm this utility implements in order to choose what window is next to any direction.

For simplicity, the following instruction assume you want to find the next window to the **right**.

 1. Begin with a list of all the currently visible windows.

 2. Discard any window that definitely is to the left. This is defined as any window whose rightmost border is to the left of the left border of the focused window.

        +------+ +------+ +------+
        |      ‖ |      | |      ‖  
        |  A   ‖ |  B*  | |  C   ‖  
        |      ‖ |      | |      ‖
        +------+ +------+ +------+  B is focused
            +------+                A is discarded
            |      ‖
            |  D   ‖
            |      ‖
            +------+

 3. Check if there are any remaining windows that share vertical range with the focused one. In that case, discard the others. Otherwise, skip this step.

        +------+          +------+
        |      |          ‖      ‖  
        |  A*  | +------+ ‖  B   ‖  
        |      | ‖      ‖ ‖      ‖
        +------+ ‖      ‖ +------+  A is focused
                 |      |           D is discarded
                 |  C   | +------+        
                 |      | |      |                      
                 |      | |  D   |                       
                 |      | |      |        
                 +------+ +------+        

 4. Of the remaining windows, pick the one whose left border is nearest to the right border of the focused window. In case of a tie where two or more windows have the same X position for their left border, pick all of them.

        +-----+ +-----+ +-----+
        |     | ‖  B  | |  C  |
        |  A* | +-----+ +-----+  A is focused
        |     | +-------------+  C is discarded
        |     | ‖      D      |
        +-----+ +-------------+

 5. If at this step we still have several windows, pick the most recently used (which is also the topmost one in the Z-stack).

In the source code the algorithm deals with generalizations of `left`, `right`, `up` and `down` in order to work in all four directions without really writing it four times.

Therefore, the *right* border is called the *agreeing border* (as it's in the same direction we intend to move), the *left* border is called the *opposite border*, the vertical range is called *perpendicular range* and *being to the right* (when right is the direction we're moving) is named *being next*. 

`directions.h` contains the concrete definition for those concepts for all four directions, which should be quite unsurprising.

Feasible improvements
---------------------

Here are some things that I would like to add to this utility but I'm not in mad need of them.

###Window clipping

Most window managers allow you to place windows on top of other windows, occluding them to the point of being invisible without being minimized. This is an issue because the user is unlikely to want to use directional movement to focus a window that is currently invisible.

This could be prevented by adding a window clipping algorithm that before starting the search algorithm would filter out any window whose rectangle is completely covered by windows on top.

This is the kind of geometric problem people write [academic papers](http://dl.acm.org/citation.cfm?id=129906) about. Fortunately there are implementations there that are small, efficient, easy to use and free. You can find one of such [implementations by Angus Johnson](http://www.angusj.com/delphi/clipper.php) at [vendor/clipper.hpp](vendor/clipper.hpp).

Currently I do most of my work in [bspwm](https://github.com/baskerville/bspwm), a tiling window manager. As such, I never have windows covering other windows so I don't really need this.

###Microsoft Windows support

There is (hopefully) nothing Linux-specific in this code, so it should be possible to add MS Windows as another window manager.

For anyone potentially pursuing this endeavour &mdash; including myself of the future, here is a list of relevant WINAPI functions:

 * [GetTopWindow](https://msdn.microsoft.com/en-us/library/windows/desktop/ms633514%28v=vs.85%29.aspx)
 * [GetNextWindow](https://msdn.microsoft.com/en-us/library/windows/desktop/ms633509%28v=vs.85%29.aspx)
 * [GetWindowRect](https://msdn.microsoft.com/en-us/library/windows/desktop/ms633519%28v=vs.85%29.aspx)
 * [BringWindowToTop](https://msdn.microsoft.com/en-us/library/windows/desktop/ms632673%28v=vs.85%29.aspx)
 * [SetWindowPos](https://msdn.microsoft.com/en-us/library/windows/desktop/ms633545%28v=vs.85%29.aspx) (it also sets size)

Hopefully it can't be harder than X11.

License
-------

MIT License.
