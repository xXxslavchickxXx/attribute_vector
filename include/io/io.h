#pragma once

#include <glm/glm.hpp>
#include <iostream>

inline std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
    os << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const glm::vec2& v) {
    os << "vec2(" << v.x << ", " << v.y << ")";
    return os;
}

inline std::ostream& operator<<(std::ostream& os, const glm::vec4& v) {
    os << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return os;
}

template<typename T>
inline std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    for (size_t i = 0; i < v.size(); i++) {
        os << v[i] << '\n';
    }
    return os;
}