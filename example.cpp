#include <iostream>
#include <attribute_vector/attribute_vector.h>
#include <glm_io/io.h>

//using namespace test;

struct A {
	//A() = delete;
};

struct SomeTag {
	using type = A;
};

int main() {
	auto a = attribute_vector<std::vector, SomeTag>();
}