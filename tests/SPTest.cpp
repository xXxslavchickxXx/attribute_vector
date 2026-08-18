#include <gtest/gtest.h>
#include <attribute_vector/attribute_vector.h>

// ========== Теги ==========
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

TEST(SingleProxyTest, BasicAccess) {
    Vector3Type vec(3);
    auto posProxy = vec.attribute<IntTag>();
    auto colProxy = vec.attribute<FloatTag>();
    auto texProxy = vec.attribute<DoubleTag>();
    
    posProxy[0] = 1;
    posProxy[1] = 2;
    posProxy[2] = 3;

    colProxy[0] = 1.1f;
    colProxy[1] = 2.2f;
    colProxy[2] = 3.3f;

    texProxy[0] = 0.1;
    texProxy[1] = 0.2;
    texProxy[2] = 0.3;

    EXPECT_EQ(posProxy[0], 1);
    EXPECT_EQ(posProxy[2], 3);
    EXPECT_EQ(colProxy[1], 2.2f);
    EXPECT_EQ(texProxy[2], 0.3);

    auto multi = vec.with<IntTag, FloatTag, DoubleTag>();
    EXPECT_EQ(multi.vector<IntTag>()[1], 2);
    EXPECT_EQ(multi.vector<FloatTag>()[0], 1.1f);
    EXPECT_EQ(multi.vector<DoubleTag>()[2], 0.3);
}

TEST(SingleProxyTest, PushBack) {
    VectorType vec;

    auto posProxy = vec.attribute<IntTag>();
    auto colProxy = vec.attribute<FloatTag>();

    posProxy.push_back(1);
    colProxy.push_back(1.1f);
    // казалось бы, по идее структура данных должна быть такой:
    // 
    // { 1, 1.1f }
    // 
    // Но на самом деле здесь происходит два push_back,
    // так как сам контейнер имеет инвариант согласованности
    // то есть - все длины одного размера, а в данном контексте
    // это значит что длины всех векторов одной длины. То есть
    // текущее состояние вектора:
    // 
    // { { 1, 0.f },
    //   { 0, 1.1f} }
    // 
    // Поэтому тесты такого формата
    // 
    // ЭТО НЕ БАГ, С ТОЧКИ ЗРЕНИЯ ЛОГИКИ ЭТО КОРРЕКТНОЕ ПОВЕДЕНИЕ
    // ИМЕННО ДЛЯ ЭТОГО ЭТА ЛИБА И СУЩЕСТВУЕТ, ЧТОБЫ НЕ БЕСПОКОИТЬСЯ
    // О СИНХРОНИЗАЦИИ РАЗМЕРОВ
    // 
    // p.s. просто будьте внимательнее
    //

    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(posProxy[0], 1);
    EXPECT_EQ(colProxy[1], 1.1f);

    posProxy.push_back(2);
    colProxy.push_back(2.2f);

    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(posProxy[2], 2);
    EXPECT_EQ(colProxy[3], 2.2f);
}

TEST(SingleProxyTest, Erase) {
    VectorType vec(3);

    auto posProxy = vec.attribute<IntTag>();
    posProxy[0] = 1;
    posProxy[1] = 2;
    posProxy[2] = 3;

    posProxy.erase(1);

    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(posProxy[0], 1);
    EXPECT_EQ(posProxy[1], 3);
}

TEST(SingleProxyTest, EraseRange) {
    VectorType vec(4);

    auto posProxy = vec.attribute<IntTag>();
    posProxy[0] = 0;
    posProxy[1] = 1;
    posProxy[2] = 2;
    posProxy[3] = 3;

    posProxy.erase(1, 2);  // удаляем индексы 1 и 2

    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(posProxy[0], 0);
    EXPECT_EQ(posProxy[1], 3);
}

TEST(SingleProxyTest, Clear) {
    VectorType vec(5);
    auto posProxy = vec.attribute<IntTag>();

    EXPECT_EQ(vec.size(), 5);

    posProxy.clear();

    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());
}

TEST(SingleProxyTest, InsertSingle) {
    VectorType vec(2);
    auto posProxy = vec.attribute<IntTag>();

    posProxy[0] = 1;
    posProxy[1] = 3;

    // Вставляем на позицию 1
    posProxy.insert(1, 2);

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(posProxy[0], 1);
    EXPECT_EQ(posProxy[1], 2);  // Вставленный
    EXPECT_EQ(posProxy[2], 3);  // Сдвинутый
}

TEST(SingleProxyTest, InsertMultiple) {
    VectorType vec(2);
    auto posProxy = vec.attribute<IntTag>();

    posProxy[0] = 0;
    posProxy[1] = 4;

    // Вставляем 3 одинаковых значения на позицию 1
    posProxy.insert(1, 3, 2);

    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(posProxy[0], 0);
    EXPECT_EQ(posProxy[1], 2);
    EXPECT_EQ(posProxy[2], 2);
    EXPECT_EQ(posProxy[3], 2);
    EXPECT_EQ(posProxy[4], 4);
}

TEST(SingleProxyTest, InsertFromContainer) {
    VectorType vec(2);
    auto posProxy = vec.attribute<IntTag>();

    posProxy[0] = 0;
    posProxy[1] = 4;

    std::vector<int> values = { 1, 2, 3 };
    posProxy.insert_containers(1, values);

    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(posProxy[0], 0);
    EXPECT_EQ(posProxy[1], 1);
    EXPECT_EQ(posProxy[2], 2);
    EXPECT_EQ(posProxy[3], 3);
    EXPECT_EQ(posProxy[4], 4);
}

TEST(SingleProxyTest, InsertFromProxy) {
    VectorType vec1(2);
    auto pos1 = vec1.attribute<IntTag>();
    pos1[0] = 1;
    pos1[1] = 3;

    VectorType vec2(1);
    auto pos2 = vec2.attribute<IntTag>();
    pos2[0] = 2;

    // Вставляем single_proxy из vec2 в vec1
    pos1.insert(1, pos2);

    EXPECT_EQ(vec1.size(), 3);
    EXPECT_EQ(pos1[0], 1);
    EXPECT_EQ(pos1[1], 2);  // Вставленный
    EXPECT_EQ(pos1[2], 3);  // Сдвинутый
}

TEST(SingleProxyTest, Upload) {
    VectorType vec(3);
    auto posProxy = vec.attribute<IntTag>();

    posProxy[0] = 1;
    posProxy[1] = 2;
    posProxy[2] = 3;

    // Перезаписываем на позиции 1
    posProxy.upload(1, 99);

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(posProxy[0], 1);
    EXPECT_EQ(posProxy[1], 99);
    EXPECT_EQ(posProxy[2], 3);
}

TEST(SingleProxyTest, UploadFromContainer) {
    VectorType vec(4);
    auto posProxy = vec.attribute<IntTag>();

    posProxy[0] = 0;
    posProxy[1] = 1;
    posProxy[2] = 2;
    posProxy[3] = 3;

    std::vector<int> new_values = { 10, 20 };

    // Перезаписываем начиная с индекса 1
    posProxy.upload_containers(1, new_values);

    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(posProxy[0], 0);
    EXPECT_EQ(posProxy[1], 10);
    EXPECT_EQ(posProxy[2], 20);
    EXPECT_EQ(posProxy[3], 3);
}

TEST(SingleProxyTest, UploadFromProxy) {
    VectorType vec1(3);
    auto pos1 = vec1.attribute<IntTag>();
    pos1[0] = 1;
    pos1[1] = 2;
    pos1[2] = 3;

    VectorType vec2(2);
    auto pos2 = vec2.attribute<IntTag>();
    pos2[0] = 20;
    pos2[1] = 30;

    // Перезаписываем начиная с индекса 1
    pos1.upload(1, pos2);

    EXPECT_EQ(vec1.size(), 3);
    EXPECT_EQ(pos1[0], 1);
    EXPECT_EQ(pos1[1], 20);
    EXPECT_EQ(pos1[2], 30);
}
 
TEST(SingleProxyTest, UploadAutoResize) {
    VectorType vec(1);
    auto posProxy = vec.attribute<IntTag>();
    posProxy[0] = 0;

    std::vector<int> new_values = { 10, 20, 30 };

    // Загружаем начиная с индекса 2
    posProxy.upload_containers(2, new_values);

    EXPECT_EQ(vec.size(), 5);  // 2 + 3 = 5
    EXPECT_EQ(posProxy[0], 0);
    EXPECT_EQ(posProxy[1], 0);
    EXPECT_EQ(posProxy[2], 10);
    EXPECT_EQ(posProxy[3], 20);
    EXPECT_EQ(posProxy[4], 30);
}

TEST(SingleProxyTest, Resize) {
    VectorType vec(2);
    auto posProxy = vec.attribute<IntTag>();
    posProxy[0] = 1;
    posProxy[1] = 2;

    posProxy.resize(5);

    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(posProxy[0], 1);
    EXPECT_EQ(posProxy[1], 2);
    EXPECT_EQ(posProxy[2], 0);
    EXPECT_EQ(posProxy[3], 0);
    EXPECT_EQ(posProxy[4], 0);

    posProxy.resize(2);
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(posProxy[0], 1);
    EXPECT_EQ(posProxy[1], 2);
}

TEST(SingleProxyTest, ResizeWithValue) {
    VectorType vec;
    auto posProxy = vec.attribute<IntTag>();

    posProxy.resize(3, 42);

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(posProxy[0], 42);
    EXPECT_EQ(posProxy[1], 42);
    EXPECT_EQ(posProxy[2], 42);
}

TEST(SingleProxyTest, Reserve) {
    VectorType vec;
    auto posProxy = vec.attribute<IntTag>();

    posProxy.reserve(100);

    EXPECT_GE(vec.capacity(), 100);
    EXPECT_EQ(vec.size(), 0);
}

TEST(SingleProxyTest, ForEach) {
    VectorType vec(3);
    auto posProxy = vec.attribute<IntTag>();

    posProxy[0] = 1;
    posProxy[1] = 2;
    posProxy[2] = 3;

    int sum = 0;
    posProxy.for_each([&](const int& value) {
        sum += value;
        });

    EXPECT_EQ(sum, 6);
}

TEST(SingleProxyTest, PopBack) {
    VectorType vec(3);
    auto posProxy = vec.attribute<IntTag>();
    posProxy[0] = 1;
    posProxy[1] = 2;
    posProxy[2] = 3;

    posProxy.pop_back();

    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(posProxy[0], 1);
    EXPECT_EQ(posProxy[1], 2);

    posProxy.pop_back();
    posProxy.pop_back();

    EXPECT_EQ(vec.size(), 0);
}

TEST(SingleProxyTest, InsertOutOfRange) {
    VectorType vec(2);
    auto posProxy = vec.attribute<IntTag>();

    // Вставка за пределами
    EXPECT_THROW(posProxy.insert(10, 42), std::out_of_range);
}

TEST(SingleProxyTest, MultipleOperations) {
    VectorType vec;
    auto posProxy = vec.attribute<IntTag>();

    // Push
    posProxy.push_back(1);
    posProxy.push_back(2);
    posProxy.push_back(3);
    EXPECT_EQ(vec.size(), 3);

    // Insert
    posProxy.insert(1, 99);
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(posProxy[1], 99);

    // Erase
    posProxy.erase(2);
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(posProxy[2], 3);

    // Resize
    posProxy.resize(5);
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(posProxy[3], 0);
    EXPECT_EQ(posProxy[4], 0);

    // Upload
    posProxy.upload(1, 77);
    EXPECT_EQ(posProxy[1], 77);

    // ForEach
    int sum = 0;
    posProxy.for_each([&](const int& v) { sum += v; });
    EXPECT_EQ(sum, 1 + 77 + 3 + 0 + 0);
}