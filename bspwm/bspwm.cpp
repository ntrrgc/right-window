//
// Created by ntrrgc on 25/01/16.
//

#include "bspwm.h"
#include "../subprocess.h"
#include <unordered_map>
#include <rapidjson/document.h>
#include <sstream>
#include <algorithm>
#include "../rw_util.h"

namespace rapidjson {
    template<typename Encoding, typename Allocator>
    typename GenericValue<Encoding, Allocator>::ValueIterator begin(
            GenericValue<Encoding, Allocator> &v) { return v.Begin(); }

    template<typename Encoding, typename Allocator>
    typename GenericValue<Encoding, Allocator>::ConstValueIterator begin(
            const GenericValue<Encoding, Allocator> &v) { return v.Begin(); }

    template<typename Encoding, typename Allocator>
    typename GenericValue<Encoding, Allocator>::ValueIterator end(
            GenericValue<Encoding, Allocator> &v) { return v.End(); }

    template<typename Encoding, typename Allocator>
    typename GenericValue<Encoding, Allocator>::ConstValueIterator end(
            const GenericValue<Encoding, Allocator> &v) { return v.End(); }
} // namespace rapidjson

namespace rw {

    class BSPWMState : public WMState {
        unordered_map<int, Window *> windows_by_id;

    public:
        BSPWMState() {
            using namespace rapidjson;

            string wm_state_json = subprocess::check_output({"bspc", "wm", "--dump-state"});
            Document wm_state_doc;

            wm_state_doc.ParseInsitu(const_cast<char *>(wm_state_json.c_str()));

            for (auto &monitor : wm_state_doc["monitors"]) {
                if (monitor["wired"].GetBool()) {
                    auto focused_desktop_name = string(monitor["focusedDesktopName"]
                                                               .GetString());

                    for (auto &desktop : monitor["desktops"]) {
                        if (desktop["name"].GetString() == focused_desktop_name) {
                            process_node(wm_state_doc, monitor, desktop, desktop["root"]);
                        }
                    }
                }
            }

            // Sort other_windows according to the stacking list
            // Most recently used last
            vector<Window*> stacking_list;
            for (auto &window_id : wm_state_doc["stackingList"]) {
                auto found = windows_by_id.find(window_id.GetInt());
                if (found != windows_by_id.end() && found->second != focused_window) {
                    stacking_list.push_back(found->second);
                }
            }
            other_windows = stacking_list;
        }


    private:
        void process_node(const rapidjson::Document &wm_state_doc,
                          const rapidjson::Value &monitor,
                          const rapidjson::Value &desktop,
                          const rapidjson::Value &node)
        {
            using namespace rapidjson;

            if (node.IsNull()) {
                return;
            }

            const Value &client = node["client"];
            if (!client.IsNull()) {
                const Value &rectangle = node["rectangle"];

                bool focused = (
                        (desktop["focusedNodeId"].GetInt() == node["id"].GetInt()) &&
                        (wm_state_doc["focusedMonitorName"].GetString() ==
                                string(monitor["name"].GetString()))
                );

                Window *window = new Window();
                window->window_id = node["id"].GetInt();
                window->top = rectangle["y"].GetInt();
                window->right = rectangle["x"].GetInt() +
                                rectangle["width"].GetInt();
                window->bottom = rectangle["y"].GetInt() +
                                 rectangle["height"].GetInt();
                window->left = rectangle["x"].GetInt();

                windows_by_id[window->window_id] = window;

                if (focused) {
                    focused_window = window;
                } else {
                    other_windows.push_back(window);
                }
            }

            process_node(wm_state_doc, monitor, desktop, node["firstChild"]);
            process_node(wm_state_doc, monitor, desktop, node["secondChild"]);
        };
    };


    unique_ptr<WMState> BSPWM::get_windows() {
        return unique_ptr<WMState>(new BSPWMState);
    }

    void BSPWM::focus(const Window *window) {
        subprocess::call({"bspc", "node", "--focus", to_string(window->window_id)});
    }

    void BSPWM::swap(const Window *a, const Window *b) {
        subprocess::call({"bspc", "node", to_string(a->window_id),
                          "--swap", to_string(b->window_id)});
    }
}