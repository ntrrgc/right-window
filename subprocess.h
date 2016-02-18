//
// Created by ntrrgc on 24/01/16.
//

#ifndef RIGHT_WINDOW_CPP_SUBPROCESS_H
#define RIGHT_WINDOW_CPP_SUBPROCESS_H

#include <string>
#include <vector>

namespace subprocess {
    std::string check_output(std::vector<std::string> args);
    void call(std::vector<std::string> args);
}

#endif //RIGHT_WINDOW_CPP_SUBPROCESS_H
