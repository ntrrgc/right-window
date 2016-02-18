#include <iostream>
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <memory>
#include "wm_abstract.h"
#include "directions.h"
#include "available_wm.h"
#include "find_next_window.h"
#include "vendor/tclap/CmdLine.h"

using namespace std;
using namespace rw;

Right RIGHT;
Left LEFT;
Up UP;
Down DOWN;

unordered_map<string, Direction*> directions = {
        { "left", &LEFT },
        { "right", &RIGHT },
        { "up", &UP },
        { "down", &DOWN },
};

template <typename D, typename M>
vector<D> map_keys(M map) {
    vector<D> ret;
    ret.reserve(map.size());
    for (auto& kv : map) {
        ret.push_back(kv.first);
    }
    return ret;
}

int main(int argc, char ** const argv) {
    using namespace TCLAP;
    Direction * direction;
    bool getId, focus, swap, debug;

    try {
        CmdLine cmd("Switches focus between windows based on cardinal directions.", ' ', "1.0");

        SwitchArg getIdSwitch("g", "get-id", "Print the identifier of the found window.");
        SwitchArg swapSwitch("s", "swap", "Swap the current window with the found window.");
        SwitchArg focusSwitch("f", "focus", "Focus the found window.");
        SwitchArg debugSwitch("d", "debug", "Dumps the information extracted from the WM.", cmd);

        vector<Arg*> xorGroup = {&getIdSwitch, &focusSwitch, &swapSwitch};
        cmd.xorAdd(xorGroup);

        vector<string> allowedDirectionNames = map_keys<string>(directions);
        auto ctr = ValuesConstraint<string>(allowedDirectionNames);
        UnlabeledValueArg<string> directionArg("direction", "The direction a window will be searched in.",
            true, "", &ctr, "DIR");

        cmd.add(directionArg);

        cmd.parse(argc, argv);

        direction = directions[directionArg.getValue()];
        getId = getIdSwitch.getValue();
        focus = focusSwitch.getValue();
        swap = swapSwitch.getValue();
        debug = debugSwitch.getValue();
    } catch (ArgException &e) {
        cerr << "Error: " << e.error() << " for argument " << e.argId() << endl;
        return 1;
    }

    unique_ptr<WindowManager> wm = get_window_manager();
    unique_ptr<WMState> wm_state = wm->get_windows();

    if (debug) {
        cout << "Focused window:" << endl << wm_state->focused_window << endl;
        cout << "Other windows:" << endl << wm_state->other_windows << endl;
    }

    Window * found_window = find_next_window(wm_state.get(), direction, debug);

    if (found_window) {
        if (focus) {
            wm->focus(found_window);
        } else if (swap) {
            wm->swap(wm_state->focused_window, found_window);
        } else if (getId) {
            cout << found_window->window_id << endl;
        }
        return 0;
    } else {
        return 1;
    }
}