#pragma once
#include <iostream>


template<typename T>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    for (size_t i = 0; i < v.size(); i++) {
        os << v[i] << '\n';
    }
    return os;
}