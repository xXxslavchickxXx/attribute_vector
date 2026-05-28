#include <iostream>
#include <attribute_vector/attribute_vector.h>
#include <glm_io/io.h>
#include <test_tags/tags.h>

int main() {
	attribute_vector<std::vector, test::Color, test::Position> vect(
		{ glm::vec4(1.f), glm::vec4(3.f), glm::vec4(-2.f) },
		{ glm::vec3(1.f), glm::vec3(3.f), glm::vec3(-2.f) }
	);
	auto base = vect.get_base<test::Position>();
	auto multi = vect.with<test::Position>();
	std::cout << base.vector<test::Position>();

	std::cout << "hello cmake\n";
}