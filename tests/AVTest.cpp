#include <gtest/gtest.h>

#include <attribute_vector/attribute_vector.h>

struct IntTag {
	using type = int;
};

struct FloatTag {
	using type = float;
};

// Тесты на конструрование ат вектора
TEST(AVConstruct, DefaultConstruct) {
	default_vector<IntTag, FloatTag> vec;
	SUCCEED();
}

TEST(AVConstruct, NConstruct) {
	default_vector<IntTag, FloatTag> vec(5);
	SUCCEED();
}

TEST(AVConstruct, InitListConstruct) {
	default_vector<IntTag, FloatTag> vec(
		{   4,   3,   2 },
		{ 0.5, 0.2, 5.3 }
	);
	SUCCEED();
}

TEST(AVConstruct, ContainerConstruct) {
	std::vector<int> intvec = { 4,   3,   2 };
	std::vector<float> floatvec = { 0.5, 0.2, 5.3 };

	default_vector<IntTag, FloatTag> vec(
		intvec, floatvec
	);
	SUCCEED();
}

// Тесты на корректное поведение
TEST(AVConstructError, NConstruct) {
    using VectorType = default_vector<IntTag, FloatTag>;

	EXPECT_THROW(
		VectorType vec(-1),
		std::length_error
	);
}