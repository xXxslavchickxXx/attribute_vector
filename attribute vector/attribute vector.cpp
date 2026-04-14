#include <iostream>
#include <attribute_vector/attribute_vector.h>
#include <io/io.h>
#include "engine/tags/tags.h"

using namespace engine;

int main()
{
    data::attribute_vector<data::VersionedVector, Position, Color> vec({glm::vec3(1.f)}, {glm::vec4(2.f)});

    std::cout << vec.with<Color, Position>();
}