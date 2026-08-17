#include <gtest/gtest.h>
#include <attribute_vector/attribute_vector.h>

struct IntTag {
    using type = int;
    static int defaultValue() { return 0; }
};

struct FloatTag {
    using type = float;
    static float defaultValue() { return 0.0f; }
};

struct DoubleTag {
    using type = double;
    static double defaultValue() { return 0.0; }
};

using VectorType = default_vector<IntTag, FloatTag>;
using Vector3Type = default_vector<IntTag, FloatTag, DoubleTag>;

TEST(SliceProxyTest, BasicSlice) {
    VectorType vec(
        { 0, 1, 2, 3, 4 },
        { 0.0f, 1.1f, 2.2f, 3.3f, 4.4f }
    );

    auto slice = vec.with<IntTag, FloatTag>().slice(1, 4);

    EXPECT_EQ(slice.size(), 3);
    EXPECT_EQ(slice.offset(), 1);

    EXPECT_EQ(slice.vector<IntTag>()[0], 1);
    EXPECT_EQ(slice.vector<IntTag>()[1], 2);
    EXPECT_EQ(slice.vector<IntTag>()[2], 3);

    EXPECT_EQ(slice.vector<FloatTag>()[0], 1.1f);
    EXPECT_EQ(slice.vector<FloatTag>()[1], 2.2f);
    EXPECT_EQ(slice.vector<FloatTag>()[2], 3.3f);
}

TEST(SliceProxyTest, EmptySlice) {
    VectorType vec(
        { 0, 1, 2 },
        { 0.0f, 1.1f, 2.2f }
    );

    auto slice = vec.with<IntTag, FloatTag>().slice(0, 0);

    EXPECT_EQ(slice.size(), 0);
    EXPECT_EQ(slice.offset(), 0);
}

TEST(SliceProxyTest, FullRangeSlice) {
    VectorType vec(
        { 0, 1, 2, 3 },
        { 0.0f, 1.1f, 2.2f, 3.3f }
    );

    auto slice = vec.with<IntTag, FloatTag>().slice(0, vec.size());

    EXPECT_EQ(slice.size(), vec.size());
    EXPECT_EQ(slice.offset(), 0);

    EXPECT_EQ(slice.vector<IntTag>()[0], 0);
    EXPECT_EQ(slice.vector<IntTag>()[3], 3);
    EXPECT_EQ(slice.vector<FloatTag>()[3], 3.3f);
}

TEST(SliceProxyTest, SliceOutOfRange) {
    VectorType vec(
        { 0, 1, 2 },
        { 0.0f, 1.1f, 2.2f }
    );

    auto proxy = vec.with<IntTag, FloatTag>();

    EXPECT_THROW(proxy.slice(3, 1), std::out_of_range);
    EXPECT_THROW(proxy.slice(1, 5), std::out_of_range);
    EXPECT_NO_THROW(proxy.slice(3, 3));
}

TEST(SliceProxyTest, DataAccess) {
    VectorType vec(
        { 0, 1, 2, 3, 4 },
        { 0.0f, 1.1f, 2.2f, 3.3f, 4.4f }
    );

    auto slice = vec.with<IntTag, FloatTag>().slice(1, 4);

    const int* int_data = slice.data<IntTag>();
    const float* float_data = slice.data<FloatTag>();

    EXPECT_EQ(int_data[0], 1);
    EXPECT_EQ(int_data[1], 2);
    EXPECT_EQ(int_data[2], 3);

    EXPECT_EQ(float_data[0], 1.1f);
    EXPECT_EQ(float_data[1], 2.2f);
    EXPECT_EQ(float_data[2], 3.3f);
}

TEST(SliceProxyTest, MutableData) {
    VectorType vec(
        { 0, 1, 2, 3, 4 },
        { 0.0f, 1.1f, 2.2f, 3.3f, 4.4f }
    );

    auto slice = vec.with<IntTag, FloatTag>().slice(1, 4);

    int* int_data = slice.data<IntTag>();
    float* float_data = slice.data<FloatTag>();

    int_data[0] = 99;
    float_data[1] = 99.9f;

    EXPECT_EQ(slice.vector<IntTag>()[0], 99);
    EXPECT_EQ(slice.vector<FloatTag>()[1], 99.9f);

    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[1], 99);
    EXPECT_EQ(vec.with<FloatTag>().vector<FloatTag>()[2], 99.9f);
}

TEST(SliceProxyTest, NestedSlice) {
    VectorType vec(
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
        { 0.0f, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f }
    );

    auto slice1 = vec.with<IntTag, FloatTag>().slice(2, 8);
    EXPECT_EQ(slice1.size(), 6);
    EXPECT_EQ(slice1.vector<IntTag>()[0], 2);
    EXPECT_EQ(slice1.vector<IntTag>()[5], 7);

    auto slice2 = slice1.slice(1, 5);
    EXPECT_EQ(slice2.size(), 4);
    EXPECT_EQ(slice2.offset(), 3);

    EXPECT_EQ(slice2.vector<IntTag>()[0], 3);
    EXPECT_EQ(slice2.vector<IntTag>()[1], 4);
    EXPECT_EQ(slice2.vector<IntTag>()[2], 5);
    EXPECT_EQ(slice2.vector<IntTag>()[3], 6);

    EXPECT_EQ(slice2.vector<FloatTag>()[0], 3.3f);
    EXPECT_EQ(slice2.vector<FloatTag>()[3], 6.6f);
}

TEST(SliceProxyTest, SliceWithThreeTags) {
    Vector3Type vec(
        { 0, 1, 2, 3, 4, 5 },
        { 0.0f, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f },
        { 0.0, 1.1, 2.2, 3.3, 4.4, 5.5 }
    );

    auto slice = vec.with<IntTag, FloatTag, DoubleTag>().slice(2, 5);

    EXPECT_EQ(slice.size(), 3);

    EXPECT_EQ(slice.vector<IntTag>()[0], 2);
    EXPECT_EQ(slice.vector<IntTag>()[1], 3);
    EXPECT_EQ(slice.vector<IntTag>()[2], 4);

    EXPECT_EQ(slice.vector<FloatTag>()[0], 2.2f);
    EXPECT_EQ(slice.vector<FloatTag>()[2], 4.4f);

    EXPECT_EQ(slice.vector<DoubleTag>()[0], 2.2);
    EXPECT_EQ(slice.vector<DoubleTag>()[2], 4.4);
}

TEST(SliceProxyTest, SliceToProxy) {
    VectorType vec(
        { 0, 1, 2, 3, 4, 5 },
        { 0.0f, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f }
    );

    auto slice = vec.with<IntTag, FloatTag>().slice(2, 5);
    auto proxy = slice.as_proxy();

    EXPECT_EQ(proxy.size(), vec.size());

    proxy.attribute<IntTag>()[3] = 99;

    EXPECT_EQ(slice.vector<IntTag>()[1], 99);
    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[3], 99);
}

TEST(SliceProxyTest, ModifyThroughSlice) {
    VectorType vec(
        { 0, 1, 2, 3, 4 },
        { 0.0f, 1.1f, 2.2f, 3.3f, 4.4f }
    );

    auto slice = vec.with<IntTag, FloatTag>().slice(1, 4);

    auto int_vec = slice.vector<IntTag>();
    int_vec[0] = 10;
    int_vec[1] = 20;
    int_vec[2] = 30;

    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[1], 10);
    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[2], 20);
    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[3], 30);
    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[0], 0);
    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[4], 4);
}

TEST(SliceProxyTest, ConstSlice) {
    VectorType vec(
        { 0, 1, 2, 3 },
        { 0.0f, 1.1f, 2.2f, 3.3f }
    );

    const auto& const_vec = vec;
    auto const_slice = const_vec.with<IntTag, FloatTag>().slice(1, 3);

    EXPECT_EQ(const_slice.vector<IntTag>()[0], 1);
    EXPECT_EQ(const_slice.vector<FloatTag>()[1], 2.2f);
}

TEST(SliceProxyTest, SliceErase) {
    VectorType vec(
        { 0, 1, 2, 3, 4, 5 },
        { 0.0f, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f }
    );

    auto slice = vec.with<IntTag, FloatTag>().slice(1, 4);
    auto proxy = slice.as_proxy();

    proxy.erase(2);

    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[0], 0);
    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[1], 1);
    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[2], 3);
    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[3], 4);
    EXPECT_EQ(vec.with<IntTag>().vector<IntTag>()[4], 5);
}

TEST(SliceProxyTest, SliceReserve) {
    VectorType vec(
        { 0, 1, 2 },
        { 0.0f, 1.1f, 2.2f }
    );

    auto slice = vec.with<IntTag, FloatTag>().slice(0, vec.size());
    auto proxy = slice.as_proxy();

    proxy.reserve(100);

    EXPECT_GE(vec.capacity(), 100);
    EXPECT_EQ(vec.size(), 3);
}

TEST(SliceProxyTest, SliceDataPointer) {
    VectorType vec(
        { 0, 1, 2, 3, 4, 5 },
        { 0.0f, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f }
    );

    auto slice = vec.with<IntTag, FloatTag>().slice(2, 5);

    const int* data_ptr = slice.data<IntTag>();
    EXPECT_EQ(data_ptr[0], 2);
    EXPECT_EQ(data_ptr[1], 3);
    EXPECT_EQ(data_ptr[2], 4);
}

TEST(SliceProxyTest, SliceConstCorrectness) {
    VectorType vec(
        { 0, 1, 2, 3 },
        { 0.0f, 1.1f, 2.2f, 3.3f }
    );

    const auto& const_vec = vec;
    auto const_slice = const_vec.with<IntTag, FloatTag>().slice(1, 3);

    const auto* int_ptr = const_slice.data<IntTag>();
    EXPECT_EQ(int_ptr[0], 1);
    EXPECT_EQ(int_ptr[1], 2);
}

TEST(SliceProxyTest, SliceBeginEnd) {
    VectorType vec(
        { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 },
        { 0.0f, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f }
    );

    auto slice = vec.with<IntTag, FloatTag>().slice(3, 8);

    EXPECT_EQ(slice.size(), 5);
    EXPECT_EQ(slice.offset(), 3);

    EXPECT_EQ(slice.vector<IntTag>()[0], 3);
    EXPECT_EQ(slice.vector<IntTag>()[4], 7);

    auto sub_slice = slice.slice(1, 4);
    EXPECT_EQ(sub_slice.size(), 3);
    EXPECT_EQ(sub_slice.offset(), 4);

    EXPECT_EQ(sub_slice.vector<IntTag>()[0], 4);
    EXPECT_EQ(sub_slice.vector<IntTag>()[2], 6);
}