#include <gtest/gtest.h>

#include <attribute_vector/attribute_vector.h>

struct IntTag {
	using type = int;
};

struct FloatTag {
	using type = float;
};
struct DoubleTag {
    using type = double;
};

using VectorType = default_vector<IntTag, FloatTag>; 

using VectorType = default_vector<IntTag, FloatTag>;
using Vector3Type = default_vector<IntTag, FloatTag, DoubleTag>;

TEST(MultiProxyTest, PushBack) {
    VectorType vec;

    auto proxy = vec.with<IntTag, FloatTag>();
    proxy.push_back(1, 6.f);

    EXPECT_EQ(vec.size(), 1);

    proxy.push_back(6, 2.3f);

    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(proxy.vector<IntTag>()[0], 1);
    EXPECT_EQ(proxy.vector<FloatTag>()[0], 6.0f);
    EXPECT_EQ(proxy.vector<IntTag>()[1], 6);
    EXPECT_EQ(proxy.vector<FloatTag>()[1], 2.3f);
}

TEST(MultiProxyTest, InsertSingle) {
    VectorType vec(
        {   0,   2 },
        { 1.f, 4.f }
    );

    auto proxy = vec.with<IntTag, FloatTag>();

    // Вставляем в середину
    proxy.insert(1,
        5,
        1.5f
    );

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(proxy.vector<IntTag>()[0], 0);
    EXPECT_EQ(proxy.vector<FloatTag>()[1], 1.5f);
    EXPECT_EQ(proxy.vector<IntTag>()[2], 2);
}

TEST(MultiProxyTest, InsertMultiple) {
    VectorType vec(
        { 0, 2 },
        { 1.f, 4.f }
    );
    auto proxy = vec.with<IntTag, FloatTag>();

    // Вставляем 3 одинаковых элемента на позицию 1
    proxy.insert(1, 3, 5, 1.5f);

    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(proxy.vector<IntTag>()[1], 5);
    EXPECT_EQ(proxy.vector<IntTag>()[2], 5);
    EXPECT_EQ(proxy.vector<IntTag>()[3], 5);
    EXPECT_EQ(proxy.vector<FloatTag>()[1], 1.5f);
    EXPECT_EQ(proxy.vector<FloatTag>()[2], 1.5f);
    EXPECT_EQ(proxy.vector<FloatTag>()[3], 1.5f);
    EXPECT_EQ(proxy.vector<IntTag>()[4], 2);
}

TEST(MultiProxyTest, InsertFromContainers) {
    VectorType vec;
    auto proxy = vec.with<IntTag, FloatTag>();

    std::vector<int> ints = { 1, 2, 3 };
    std::vector<float> floats = { 1.1f, 2.2f, 3.3f };

    proxy.insert_containers(0, ints, floats);

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(proxy.vector<IntTag>()[0], 1);
    EXPECT_EQ(proxy.vector<IntTag>()[1], 2);
    EXPECT_EQ(proxy.vector<IntTag>()[2], 3);
    EXPECT_EQ(proxy.vector<FloatTag>()[0], 1.1f);
    EXPECT_EQ(proxy.vector<FloatTag>()[1], 2.2f);
    EXPECT_EQ(proxy.vector<FloatTag>()[2], 3.3f);
}

TEST(MultiProxyTest, InsertFromProxy) {
    // Вектор с 3 тегами
    Vector3Type vec1(
        { 0, 1 },
        { 1.1f, 2.2f },
        { 0.0, 1.0 }
    );
    auto proxy1 = vec1.with<IntTag, FloatTag, DoubleTag>();

    // Вектор только с 2 тегами
    VectorType vec2(
        { 5, 6 },
        { 5.5f, 6.6f }
    );
    auto proxy2 = vec2.with<IntTag, FloatTag>();

    // Вставляем vec2 в vec1
    proxy1.insert(1, proxy2);

    EXPECT_EQ(vec1.size(), 4);
    EXPECT_EQ(proxy1.vector<IntTag>()[0], 0);
    EXPECT_EQ(proxy1.vector<IntTag>()[1], 5);
    EXPECT_EQ(proxy1.vector<IntTag>()[2], 6);
    EXPECT_EQ(proxy1.vector<IntTag>()[3], 1);
    EXPECT_EQ(proxy1.vector<DoubleTag>()[1], 0.0);
    EXPECT_EQ(proxy1.vector<DoubleTag>()[2], 0.0);
}

TEST(MultiProxyTest, Upload) {
    VectorType vec(
        { 0, 1, 2 },
        { 1.1f, 2.2f, 3.3f }
    );
    auto proxy = vec.with<IntTag, FloatTag>();

    // Перезаписываем значение на позиции 1
    proxy.upload(1, 99, 9.9f);

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(proxy.vector<IntTag>()[0], 0);
    EXPECT_EQ(proxy.vector<IntTag>()[1], 99);
    EXPECT_EQ(proxy.vector<IntTag>()[2], 2);
    EXPECT_EQ(proxy.vector<FloatTag>()[1], 9.9f);
}

TEST(MultiProxyTest, UploadFromContainers) {
    VectorType vec(
        { 0, 1, 2, 3 },
        { 1.1f, 2.2f, 3.3f, 4.4f }
    );
    auto proxy = vec.with<IntTag, FloatTag>();

    std::vector<int> new_ints = { 10, 20 };
    std::vector<float> new_floats = { 10.1f, 20.2f };

    // Перезаписываем начиная с позиции 1
    proxy.upload_containers(1, new_ints, new_floats);

    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(proxy.vector<IntTag>()[0], 0);
    EXPECT_EQ(proxy.vector<IntTag>()[1], 10);
    EXPECT_EQ(proxy.vector<IntTag>()[2], 20);
    EXPECT_EQ(proxy.vector<IntTag>()[3], 3);
    EXPECT_EQ(proxy.vector<FloatTag>()[1], 10.1f);
    EXPECT_EQ(proxy.vector<FloatTag>()[2], 20.2f);
}

TEST(MultiProxyTest, UploadAutoResize) {
    VectorType vec(
        { 0 },
        { 1.1f }
    );
    auto proxy = vec.with<IntTag, FloatTag>();

    std::vector<int> new_ints = { 10, 20, 30 };
    std::vector<float> new_floats = { 10.1f, 20.2f, 30.3f };

    // Загружаем начиная с позиции 2 (вектор расширится)
    proxy.upload_containers(2, new_ints, new_floats);

    EXPECT_EQ(vec.size(), 5);  // 2 + 3 = 5
    EXPECT_EQ(proxy.vector<IntTag>()[0], 0);
    EXPECT_EQ(proxy.vector<IntTag>()[1], 0);
    EXPECT_EQ(proxy.vector<IntTag>()[2], 10);
    EXPECT_EQ(proxy.vector<IntTag>()[3], 20);
    EXPECT_EQ(proxy.vector<IntTag>()[4], 30);
}

TEST(MultiProxyTest, Erase) {
    VectorType vec(
        { 0, 1, 2 },
        { 1.1f, 2.2f, 3.3f }
    );
    auto proxy = vec.with<IntTag, FloatTag>();

    proxy.erase(1);

    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(proxy.vector<IntTag>()[0], 0);
    EXPECT_EQ(proxy.vector<IntTag>()[1], 2);
    EXPECT_EQ(proxy.vector<FloatTag>()[1], 3.3f);
}

TEST(MultiProxyTest, EraseRange) {
    VectorType vec(
        { 0, 1, 2, 3 },
        { 1.1f, 2.2f, 3.3f, 4.4f }
    );
    auto proxy = vec.with<IntTag, FloatTag>();

    proxy.erase(1, 2);  // удаляем элементы 1 и 2

    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(proxy.vector<IntTag>()[0], 0);
    EXPECT_EQ(proxy.vector<IntTag>()[1], 3);
    EXPECT_EQ(proxy.vector<FloatTag>()[1], 4.4f);
}

TEST(MultiProxyTest, EraseOutOfRange) {
    VectorType vec;
    auto proxy = vec.with<IntTag, FloatTag>();

    // Попытка стереть из пустого вектора
    EXPECT_THROW(proxy.erase(0), std::out_of_range);

    proxy.push_back(1, 1.1f);

    // Попытка стереть за пределами
    EXPECT_THROW(proxy.erase(5), std::out_of_range);
}

TEST(MultiProxyTest, Clear) {
    VectorType vec(
        { 0, 1, 2 },
        { 1.1f, 2.2f, 3.3f }
    );
    auto proxy = vec.with<IntTag, FloatTag>();

    proxy.clear();

    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());
}

TEST(MultiProxyTest, ResizeLarger) {
    VectorType vec(
        { 0 },
        { 1.1f }
    );
    auto proxy = vec.with<IntTag, FloatTag>();

    proxy.resize(3);

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(proxy.vector<IntTag>()[0], 0);
    EXPECT_EQ(proxy.vector<IntTag>()[1], 0);
    EXPECT_EQ(proxy.vector<IntTag>()[2], 0);
    EXPECT_EQ(proxy.vector<FloatTag>()[1], 0.0f);
}

TEST(MultiProxyTest, ResizeSmaller) {
    VectorType vec(
        { 0, 1, 2 },
        { 1.1f, 2.2f, 3.3f }
    );
    auto proxy = vec.with<IntTag, FloatTag>();

    proxy.resize(1);

    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(proxy.vector<IntTag>()[0], 0);
    EXPECT_EQ(proxy.vector<FloatTag>()[0], 1.1f);
}

TEST(MultiProxyTest, ResizeWithValues) {
    VectorType vec;
    auto proxy = vec.with<IntTag, FloatTag>();

    proxy.resize(2, 42, 3.14f);

    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(proxy.vector<IntTag>()[0], 42);
    EXPECT_EQ(proxy.vector<IntTag>()[1], 42);
    EXPECT_EQ(proxy.vector<FloatTag>()[0], 3.14f);
    EXPECT_EQ(proxy.vector<FloatTag>()[1], 3.14f);
}

TEST(MultiProxyTest, Reserve) {
    VectorType vec;
    auto proxy = vec.with<IntTag, FloatTag>();

    proxy.reserve(100);

    EXPECT_GE(vec.capacity(), 100);
    EXPECT_EQ(vec.size(), 0);
}

TEST(MultiProxyTest, PopBack) {
    VectorType vec(
        { 0, 1 },
        { 1.1f, 2.2f }
    );
    auto proxy = vec.with<IntTag, FloatTag>();

    proxy.pop_back();

    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(proxy.vector<IntTag>()[0], 0);
    EXPECT_EQ(proxy.vector<FloatTag>()[0], 1.1f);
}

// ========== PopBack Empty ==========
TEST(MultiProxyTest, PopBackEmpty) {
    VectorType vec;
    auto proxy = vec.with<IntTag, FloatTag>();

    // pop_back на пустом векторе - неопределенное поведение или исключение?
    // Обычно вызывает неопределенное поведение, но у тебя может быть проверка
    // Если есть проверка - раскомментируй:
    // EXPECT_THROW(proxy.pop_back(), std::out_of_range);
}

TEST(MultiProxyTest, MultipleOperations) {
    VectorType vec;
    auto proxy = vec.with<IntTag, FloatTag>();

    // Вставляем
    proxy.push_back(1, 1.1f);
    proxy.push_back(2, 2.2f);
    proxy.push_back(3, 3.3f);
    EXPECT_EQ(vec.size(), 3);

    // Вставляем в середину
    proxy.insert(1, 99, 9.9f);
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(proxy.vector<IntTag>()[1], 99);

    // Удаляем
    proxy.erase(2);
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(proxy.vector<IntTag>()[2], 3);

    // Меняем размер
    proxy.resize(5);
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(proxy.vector<IntTag>()[3], 0);
    EXPECT_EQ(proxy.vector<IntTag>()[4], 0);
}