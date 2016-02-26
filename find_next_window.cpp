//
// Created by ntrrgc on 26/01/16.
//

#include "find_next_window.h"
#include "directions.h"
#include "window_clipping.h"
#include <cassert>
#include <algorithm>
#include <iostream>
#include <tuple>

using namespace std;

namespace rw {

    enum Border {
        BORDER_OPPOSITE,
        BORDER_AGREEING,
    };

    int overlap_length(tuple<int,int> range_a, tuple<int,int> range_b) {
        // a must start before or on the same point as b
        if (get<0>(range_a) > get<0>(range_b)) {
            return overlap_length(range_b, range_a);
        }

        if (get<0>(range_b) > get<1>(range_a)) {
            return 0; // no overlap
        } else {
            int ret = min(get<1>(range_a), get<1>(range_b)) - get<0>(range_b);
            assert(ret >= 0);
            return ret;
        }
    }

    vector<Window*> window_with_border_next_and_nearest(
            Window* focused_window,
            const vector<Window*> other_windows,
            Direction * direction,
            Border border)
    {
        function<int(const Window*)> border_of;
        if (border == BORDER_OPPOSITE) {
            border_of = [direction](const Window* window) {
                return direction->opposite_border(window);
            };
        } else if (border == BORDER_AGREEING) {
            border_of = [direction](const Window* window) {
                return direction->agreeing_border(window);
            };
        } else {
            throw new runtime_error("Invalid border");
        }

        int focused_position = border_of(focused_window);

        bool best_other_position_set = false;
        int best_other_position;
        vector<Window*> best_others;

        for (auto& other : other_windows) {
            int other_position = border_of(other);
            if (direction->next_to(other_position, focused_position) && (
                    !best_other_position_set ||
                    direction->prev_or_equal_to(other_position, best_other_position)))
            {
                if (!best_other_position_set || best_other_position != other_position) {
                    best_other_position_set = true;
                    best_other_position = other_position;
                    best_others.clear();
                }
                best_others.push_back(other);
            }
        }

        return best_others;
    }

    vector<Window*> window_with_greatest_perpendicular_overlap(
            Window* focused_window,
            const vector<Window*> other_windows,
            Direction * direction)
    {
        auto focused_range = direction->perpendicular_range(focused_window);

        bool best_overlap_set = false;
        int best_overlap;
        vector<Window*> best_windows;

        for (auto& other : other_windows) {
            auto other_range = direction->perpendicular_range(other);

            int overlap = overlap_length(focused_range, other_range);
            // Note: small overlap differences as treated as if there was no
            // difference. This is because in practice these small differences
            // are side effects of splitting the screen when the height or width
            // is not even.
            const int small_difference = 50;

            if (!best_overlap_set || (overlap - best_overlap > small_difference)) {
                best_overlap_set = true;
                best_overlap = overlap;
                best_windows.clear();
            }

            if (abs(overlap - best_overlap) <= small_difference) {
                best_windows.push_back(other);
            }
        }

        return best_windows;
    }

    vector<Window*> discard_windows_other_direction(
            Window* focused_window,
            const vector<Window*> other_windows,
            Direction * direction)
    {
        int focused_end = direction->agreeing_border(focused_window);

        vector<Window*> selected_windows;

        for (auto& other : other_windows) {
            int other_end = direction->agreeing_border(other);

            if (direction->next_to(other_end, focused_end)) {
                selected_windows.push_back(other);
            }
        }

        return selected_windows;
    }

    vector<Window*> windows_sharing_perpendicular_range(
            Window* focused_window,
            const vector<Window*> other_windows,
            Direction * direction)
    {
        auto focused_range = direction->perpendicular_range(focused_window);

        vector<Window*> selected_windows;

        for (auto& other : other_windows) {
            auto other_range = direction->perpendicular_range(other);

            int overlap = overlap_length(focused_range, other_range);
            if (overlap > 0) {
                selected_windows.push_back(other);
            }
        }

        return selected_windows;
    }

    Window *find_next_window(WMState * wm, Direction * direction, bool debug) {
        if (!wm->focused_window || wm->other_windows.size() == 0) {
            return nullptr; // Nothing to do here!
        }

        // For the sake of familiarity these comments will assume direction ==
        // right, but the code is generic instead to
        // handle other directions.

        // Generalizations (assuming direction == right):
        //   to the right of X -> next to X
        //   left border -> opposite border
        //   right border -> agreeing border
        //   vertical overlap -> perpendicular overlap

        vector<Window*> other_windows = wm->other_windows;
        if (debug) {
            cout << "Start:\n" << other_windows << endl;
        }

        // Step 1.
        // Begin with a list of all the currently visible windows.
        // Windows totally hidden behind other windows should be filtered out.
        // This is currently not implemented though, so it's a no-op.

        other_windows = window_clipping(wm->focused_window, wm->other_windows);
        if (debug) {
            cout << "Filter 1:\n" << other_windows << endl;
        }

        // Step 2.
        // Discard any window that definitely is to the left. This is defined as
        // any window whose rightmost border is to
        // the left of the left border of the focused window.

        //  +------+ +------+ +------+
        //  |      ‖ |      | |      ‖  
        //  |  A   ‖ |  B*  | |  C   ‖  
        //  |      ‖ |      | |      ‖
        //  +------+ +------+ +------+  B is focused
        //      +------+                A is discarded
        //      |      ‖
        //      |  D   ‖
        //      |      ‖
        //      +------+

        other_windows = discard_windows_other_direction(
                wm->focused_window, other_windows, direction);
        if (debug) {
            cout << "Filter 2:\n" << other_windows << endl;
        }

        // Step 3.
        // Check if there are any remaining windows that share vertical range
        // with the focused one. In that case,
        // discard the others. Otherwise, skip this step.

        //  +------+          +------+
        //  |      |          ‖      ‖  
        //  |  A*  | +------+ ‖  B   ‖  
        //  |      | ‖      ‖ ‖      ‖
        //  +------+ ‖      ‖ +------+  A is focused
        //           |      |           D is discarded
        //           |  C   | +------+        
        //           |      | |      |                      
        //           |      | |  D   |                       
        //           |      | |      |        
        //           +------+ +------+        

        vector<Window*> windows_overlapping = windows_sharing_perpendicular_range(
                wm->focused_window, other_windows, direction);

        if (windows_overlapping.size() > 0) {
            other_windows = windows_overlapping;
        }
        if (debug) {
            if (windows_overlapping.size() > 0) {
                cout << "Filter 3:\n" << other_windows << endl;
            } else {
                cout << "Filter 3:\n" << "(skipped)" << endl;
            }
        }

        // Step 4.
        // Of the remaining windows, pick the one whose left border is nearest
        // to the right border of the focused window. In case of a tie where two
        // or more windows have the same X position for their left border, pick
        // all of them.

        //  +-----+ +-----+ +-----+
        //  |     | ‖  B  | |  C  |
        //  |  A* | +-----+ +-----+  A is focused
        //  |     | +-------------+  C is discarded
        //  |     | ‖      D      |
        //  +-----+ +-------------+

        other_windows = window_with_border_next_and_nearest(
                wm->focused_window, other_windows, direction, BORDER_OPPOSITE);
        if (debug) {
            cout << "Filter 4:\n" << other_windows << endl;
        }

        // Step 5.
        // If at this step we still have several windows, pick the most recently
        // used (which is also the topmost one in the Z-stack).
        // Note that as the other_windows vector is sorted by Z-index we just
        // pick the last element.

        if (other_windows.size() > 0) {
            return other_windows[other_windows.size() - 1];
        } else {
            // No window found.
            return nullptr;
        }
    }
}
