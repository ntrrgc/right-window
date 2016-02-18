//
// Created by ntrrgc on 26/01/16.
//

#ifndef RIGHT_WINDOW_CPP_UTIL_H
#define RIGHT_WINDOW_CPP_UTIL_H

template<typename T, typename V>
long indexOf(T container, V value) {
    auto iter = find(container.begin(), container.end(), value);
    assert(iter != container.end());
    return distance(container.begin(), iter);
}

#endif //RIGHT_WINDOW_CPP_UTIL_H
