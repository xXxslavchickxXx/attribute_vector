#include <gtest/gtest.h>

#include <attribute_vector/attribute_vector.h>

struct IntTag {
	using type = int;
};

struct FloatTag {
	using type = float;
};

TEST(BaseProxyTest, SizeMatch) {
	default_vector<IntTag, FloatTag> vec(5);
	auto base = vec.get_base<IntTag, FloatTag>();

	EXPECT_EQ(base.size(), 5);
}