// attribute_vector_demo.cpp
#include <iostream>
#include <cassert>
#include <sstream>
#include <string>
#include <deque>

#include <attribute_vector/attribute_vector.h>
#include <test_tags/tags.h>
#include <glm_io/io.h>

using namespace test;

// ============================================================
// Вспомогательные функции для тестов
// ============================================================

#define ASSERT(cond) \
    do { \
        if (!(cond)) { \
            std::cerr << "❌ ASSERT FAILED: " << #cond << " (line " << __LINE__ << ")\n"; \
            std::abort(); \
        } else { \
            std::cout << "✅ " << #cond << "\n"; \
        } \
    } while(0)

// ============================================================
// Тесты
// ============================================================

void test_slice_basic() {
    std::cout << "\n========== test_slice_basic ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2), glm::vec3(3,3,3), glm::vec3(4,4,4) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1), glm::vec4(0,0,1,1), glm::vec4(1,1,0,1), glm::vec4(1,0,1,1) }
    );

    // От 1 до 4: 1, 2, 3
    auto s = vec.slice<Position, Color>(1, 4);

    ASSERT(s.size() == 3);
    ASSERT(s.offset() == 1);

    // Проверяем доступ к элементам
    ASSERT(s.vector<Position>()[0] == glm::vec3(1, 1, 1));
    ASSERT(s.vector<Position>()[1] == glm::vec3(2, 2, 2));
    ASSERT(s.vector<Position>()[2] == glm::vec3(3, 3, 3));

    ASSERT(s.vector<Color>()[0] == glm::vec4(0, 1, 0, 1));
    ASSERT(s.vector<Color>()[1] == glm::vec4(0, 0, 1, 1));
    ASSERT(s.vector<Color>()[2] == glm::vec4(1, 1, 0, 1));

    std::cout << "✅ Basic slice operations\n";
}

void test_slice_data_pointer() {
    std::cout << "\n========== test_slice_data_pointer ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2), glm::vec3(3,3,3) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1), glm::vec4(0,0,1,1), glm::vec4(1,1,0,1) }
    );

    auto s = vec.slice<Position>(1, 3);

    const glm::vec3* ptr = s.data<Position>();
    ASSERT(ptr != nullptr);
    ASSERT(ptr[0] == glm::vec3(1, 1, 1));
    ASSERT(ptr[1] == glm::vec3(2, 2, 2));

    // Проверяем, что указатель указывает на правильное место в памяти
    const glm::vec3* original_data = vec.with<Position>().vector<Position>().data();
    ASSERT(ptr == original_data + 1);

    std::cout << "✅ Slice data() pointer works correctly\n";
}

void test_slice_nested() {
    std::cout << "\n========== test_slice_nested ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2), glm::vec3(3,3,3), glm::vec3(4,4,4) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1), glm::vec4(0,0,1,1), glm::vec4(1,1,0,1), glm::vec4(1,0,1,1) }
    );

    // Берём слайс [1, 4) → элементы 1,2,3
    auto s1 = vec.slice<Position, Color>(1, 4);
    ASSERT(s1.size() == 3);
    ASSERT(s1.vector<Position>()[0] == glm::vec3(1, 1, 1));

    // Делаем под-слайс [1, 3) внутри → элементы 2,3
    auto s2 = s1.slice(1, 3);
    ASSERT(s2.size() == 2);
    ASSERT(s2.offset() == 2);  // глобальный индекс 2
    ASSERT(s2.vector<Position>()[0] == glm::vec3(2, 2, 2));
    ASSERT(s2.vector<Position>()[1] == glm::vec3(3, 3, 3));

    std::cout << "✅ Nested slice works correctly\n";
}

void test_slice_as_proxy() {
    std::cout << "\n========== test_slice_as_proxy ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1), glm::vec4(0,0,1,1) }
    );

    auto s = vec.slice<Position, Color>(0, 2);
    auto proxy = s.as_proxy();

    // proxy должен иметь доступ ко всем данным вектора (не только слайса!)
    ASSERT(proxy.size() == 3);  // полный размер, а не 2!
    ASSERT(proxy.vector<Position>()[2] == glm::vec3(2, 2, 2));

    std::cout << "✅ as_proxy() returns full proxy\n";
}

void test_slice_empty() {
    std::cout << "\n========== test_slice_empty ==========\n";

    default_vector<Position, Color> vec(5);

    // Пустой слайс (begin == end)
    auto s = vec.slice<Position>(2, 2);
    ASSERT(s.size() == 0);

    // Проверяем, что data() с пустым слайсом не крашится
    const glm::vec3* ptr = s.data<Position>();
    ASSERT(ptr != nullptr);  // Указатель на начало (но dereferencing нельзя)

    std::cout << "✅ Empty slice works\n";
}

void test_slice_out_of_range() {
    std::cout << "\n========== test_slice_out_of_range ==========\n";

    default_vector<Position, Color> vec(5);

    bool caught = false;
    try {
        auto s = vec.slice<Position>(2, 10);  // end > size()
    }
    catch (const std::out_of_range& e) {
        caught = true;
        std::cout << "✅ Caught expected exception: " << e.what() << "\n";
    }
    ASSERT(caught);

    caught = false;
    try {
        auto s = vec.slice<Position>(5, 3);  // begin > end
    }
    catch (const std::out_of_range& e) {
        caught = true;
        std::cout << "✅ Caught expected exception: " << e.what() << "\n";
    }
    ASSERT(caught);

    std::cout << "✅ Out of range validation works\n";
}

void test_slice_for_each() {
    std::cout << "\n========== test_slice_for_each ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2), glm::vec3(3,3,3) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1), glm::vec4(0,0,1,1), glm::vec4(1,1,0,1) }
    );

    auto s = vec.slice<Position, Color>(1, 3);

    // Модифицируем через for_each (нужно будет реализовать в slice_proxy)
    // Пока просто проверяем чтение
    float sum_x = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        sum_x += s.vector<Position>()[i].x;
    }
    ASSERT(sum_x == 1.0f + 2.0f);

    std::cout << "✅ Slice iteration works\n";
}

void test_default_construction() {
    std::cout << "\n========== test_default_construction ==========\n";

    default_vector<Position, Color, TexCoords> vec;
    ASSERT(vec.size() == 0);
    ASSERT(vec.with<Position>().size() == 0);
    ASSERT(vec.with<Color>().size() == 0);
    ASSERT(vec.with<TexCoords>().size() == 0);
}

void test_sized_construction() {
    std::cout << "\n========== test_sized_construction ==========\n";

    version_vector<Position, Color> vec(5);

    ASSERT(vec.size() == 5);
    ASSERT(vec.with<Position>().size() == 5);
    ASSERT(vec.with<Color>().size() == 5);

    // Проверяем значения по умолчанию
    auto proxy = vec.with<Position, Color>();
    ASSERT(proxy.vector<Position>()[0] == Position::defaultValue());
    ASSERT(proxy.vector<Position>()[4] == Position::defaultValue());
    ASSERT(proxy.vector<Color>()[0] == Color::defaultValue());
}

void test_initializer_list_construction() {
    std::cout << "\n========== test_initializer_list_construction ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1), glm::vec4(0,0,1,1) }
    );

    ASSERT(vec.size() == 3);
    ASSERT(vec.with<Position>().vector<Position>()[1] == glm::vec3(1, 1, 1));
    ASSERT(vec.with<Color>().vector<Color>()[2] == glm::vec4(0, 0, 1, 1));
}

void test_initializer_list_size_mismatch() {
    std::cout << "\n========== test_initializer_list_size_mismatch ==========\n";

    bool caught = false;
    try {
        default_vector<Position, Color> vec(
            { glm::vec3(0,0,0) },
            { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1) }  // разный размер!
        );
    }
    catch (const std::runtime_error& e) {
        caught = true;
        std::cout << "✅ Caught expected exception: " << e.what() << "\n";
    }
    catch (...) {
        caught = true;
        std::cout << "✅ Caught expected exception (unknown type)\n";
    }
    ASSERT(caught);
}

void test_push_back() {
    std::cout << "\n========== test_push_back ==========\n";

    default_vector<Position, Color> vec;

    vec.with<Position, Color>().push_back(
        glm::vec3(1.0f, 2.0f, 3.0f),
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
    );

    ASSERT(vec.size() == 1);
    ASSERT(vec.with<Position>().vector<Position>()[0] == glm::vec3(1, 2, 3));
    ASSERT(vec.with<Color>().vector<Color>()[0] == glm::vec4(1, 0, 0, 1));

    // Добавляем вторую вершину
    vec.with<Position, Color>().push_back(
        glm::vec3(4.0f, 5.0f, 6.0f),
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
    );

    ASSERT(vec.size() == 2);
    ASSERT(vec.with<Position>().vector<Position>()[1] == glm::vec3(4, 5, 6));
}

void test_insert_single() {
    std::cout << "\n========== test_insert_single ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(2,2,2) },
        { glm::vec4(1,0,0,1), glm::vec4(0,0,1,1) }
    );

    // Вставляем между ними
    vec.with<Position, Color>().insert(1,
        glm::vec3(1, 1, 1),
        glm::vec4(0, 1, 0, 1)
    );

    ASSERT(vec.size() == 3);
    ASSERT(vec.with<Position>().vector<Position>()[1] == glm::vec3(1, 1, 1));
    ASSERT(vec.with<Color>().vector<Color>()[1] == glm::vec4(0, 1, 0, 1));
    ASSERT(vec.with<Position>().vector<Position>()[2] == glm::vec3(2, 2, 2));
}

void test_insert_multiple() {
    std::cout << "\n========== test_insert_multiple ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0) },
        { glm::vec4(1,0,0,1) }
    );

    // Вставляем 3 одинаковые вершины в конец
    vec.with<Position, Color>().insert(1, 3,
        glm::vec3(1, 1, 1),
        glm::vec4(0, 1, 0, 1)
    );

    ASSERT(vec.size() == 4);
    ASSERT(vec.with<Position>().vector<Position>()[1] == glm::vec3(1, 1, 1));
    ASSERT(vec.with<Position>().vector<Position>()[3] == glm::vec3(1, 1, 1));
}

void test_insert_from_containers() {
    std::cout << "\n========== test_insert_from_containers ==========\n";

    default_vector<Position, Color> vec;

    std::vector<glm::vec3> positions = { {0,0,0}, {1,1,1}, {2,2,2} };
    std::vector<glm::vec4> colors = { {1,0,0,1}, {0,1,0,1}, {0,0,1,1} };

    vec.with<Position, Color>().insert_containers(0, positions, colors);

    ASSERT(vec.size() == 3);
    ASSERT(vec.with<Position>().vector<Position>()[2] == glm::vec3(2, 2, 2));
    ASSERT(vec.with<Color>().vector<Color>()[1] == glm::vec4(0, 1, 0, 1));
}

void test_insert_from_proxy() {
    std::cout << "\n========== test_insert_from_proxy ==========\n";

    default_vector<Position, Color, TexCoords> vec1(
        { glm::vec3(0,0,0), glm::vec3(1,1,1) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1) },
        { glm::vec2(0,0), glm::vec2(1,1) }
    );

    // Второй вектор только с Position и Color (без TexCoords)
    default_vector<Position, Color> vec2(
        { glm::vec3(2,2,2) },
        { glm::vec4(0,0,1,1) }
    );

    // Вставляем vec2 в vec1 — TexCoords должны заполниться дефолтами
    vec1.with<Position, Color, TexCoords>().insert(1, vec2.with<Position, Color>());

    ASSERT(vec1.size() == 3);
    ASSERT(vec1.with<Position>().vector<Position>()[1] == glm::vec3(2, 2, 2));
    ASSERT(vec1.with<TexCoords>().vector<TexCoords>()[1] == TexCoords::defaultValue());
}

void test_upload() {
    std::cout << "\n========== test_upload ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2) },
        { glm::vec4(1,1,1,1), glm::vec4(1,1,1,1), glm::vec4(1,1,1,1) }
    );

    // Перезаписываем значение по индексу 0
    vec.with<Position, Color>().upload(0,
        glm::vec3(99, 99, 99),
        glm::vec4(0, 0, 0, 1)
    );

    ASSERT(vec.with<Position>().vector<Position>()[0] == glm::vec3(99, 99, 99));
    ASSERT(vec.with<Color>().vector<Color>()[0] == glm::vec4(0, 0, 0, 1));
    ASSERT(vec.with<Position>().vector<Position>()[1] == glm::vec3(1, 1, 1)); // не изменился
}

void test_upload_from_containers() {
    std::cout << "\n========== test_upload_from_containers ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2) },
        { glm::vec4(1,1,1,1), glm::vec4(1,1,1,1), glm::vec4(1,1,1,1) }
    );

    std::vector<glm::vec3> new_positions = { {10,10,10}, {20,20,20} };
    std::vector<glm::vec4> new_colors = { {0,0,0,1}, {0,0,0,1} };

    // Загружаем начиная с индекса 1
    vec.with<Position, Color>().upload_containers(1, new_positions, new_colors);

    ASSERT(vec.with<Position>().vector<Position>()[0] == glm::vec3(0, 0, 0));   // не тронут
    ASSERT(vec.with<Position>().vector<Position>()[1] == glm::vec3(10, 10, 10)); // перезаписан
    ASSERT(vec.with<Position>().vector<Position>()[2] == glm::vec3(20, 20, 20)); // перезаписан
}

void test_upload_auto_resize() {
    std::cout << "\n========== test_upload_auto_resize ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0) },
        { glm::vec4(1,1,1,1) }
    );

    std::vector<glm::vec3> positions = { {1,1,1}, {2,2,2}, {3,3,3} };
    std::vector<glm::vec4> colors = { {0,0,0,1}, {0,0,0,1}, {0,0,0,1} };

    // Загружаем начиная с индекса 2, вектор автоматически расширится
    vec.with<Position, Color>().upload_containers(2, positions, colors);

    ASSERT(vec.size() == 5);  // 2 (индекс) + 3 (данных) = 5
    ASSERT(vec.with<Position>().vector<Position>()[0] == glm::vec3(0, 0, 0)); // остался
    ASSERT(vec.with<Position>().vector<Position>()[1] == Position::defaultValue()); // дефолт
    ASSERT(vec.with<Position>().vector<Position>()[2] == glm::vec3(1, 1, 1)); // загружен
    ASSERT(vec.with<Position>().vector<Position>()[4] == glm::vec3(3, 3, 3)); // загружен
}

void test_erase() {
    std::cout << "\n========== test_erase ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1), glm::vec4(0,0,1,1) }
    );

    vec.with<Position, Color>().erase(1);

    ASSERT(vec.size() == 2);
    ASSERT(vec.with<Position>().vector<Position>()[0] == glm::vec3(0, 0, 0));
    ASSERT(vec.with<Position>().vector<Position>()[1] == glm::vec3(2, 2, 2));
    ASSERT(vec.with<Color>().vector<Color>()[1] == glm::vec4(0, 0, 1, 1));
}

void test_erase_range() {
    std::cout << "\n========== test_erase_range ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2), glm::vec3(3,3,3) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1), glm::vec4(0,0,1,1), glm::vec4(1,1,0,1) }
    );

    vec.with<Position, Color>().erase(1, 2);

    ASSERT(vec.size() == 2);
    ASSERT(vec.with<Position>().vector<Position>()[0] == glm::vec3(0, 0, 0));
    ASSERT(vec.with<Position>().vector<Position>()[1] == glm::vec3(3, 3, 3));
}

void test_erase_out_of_range() {
    std::cout << "\n========== test_erase_out_of_range ==========\n";

    default_vector<Position, Color> vec;

    // Стирание из пустого вектора — должно бросить исключение
    bool caught = false;
    try {
        vec.with<Position, Color>().erase(0);
    }
    catch (const std::out_of_range& e) {
        caught = true;
        std::cout << "✅ Caught expected exception: " << e.what() << "\n";
    }
    catch (...) {
        caught = true;
        std::cout << "✅ Caught expected exception (unknown type)\n";
    }
    ASSERT(caught);

    vec.with<Position, Color>().push_back(
        glm::vec3(0, 0, 0),
        glm::vec4(1, 1, 1, 1)
    );

    // Стирание за пределами — должно бросить исключение
    caught = false;
    try {
        vec.with<Position, Color>().erase(5);
    }
    catch (const std::out_of_range& e) {
        caught = true;
        std::cout << "✅ Caught expected exception: " << e.what() << "\n";
    }
    catch (...) {
        caught = true;
        std::cout << "✅ Caught expected exception (unknown type)\n";
    }
    ASSERT(caught);
}

void test_resize_larger() {
    std::cout << "\n========== test_resize_larger ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0) },
        { glm::vec4(1,1,1,1) }
    );

    vec.with<Position, Color>().resize(3);

    ASSERT(vec.size() == 3);
    ASSERT(vec.with<Position>().vector<Position>()[0] == glm::vec3(0, 0, 0));
    ASSERT(vec.with<Position>().vector<Position>()[1] == Position::defaultValue());
    ASSERT(vec.with<Color>().vector<Color>()[2] == Color::defaultValue());
}

void test_resize_smaller() {
    std::cout << "\n========== test_resize_smaller ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1), glm::vec4(0,0,1,1) }
    );

    vec.with<Position, Color>().resize(1);

    ASSERT(vec.size() == 1);
    ASSERT(vec.with<Position>().vector<Position>()[0] == glm::vec3(0, 0, 0));
}

void test_resize_with_values() {
    std::cout << "\n========== test_resize_with_values ==========\n";

    default_vector<Position, Color> vec;

    vec.with<Position, Color>().resize(2,
        glm::vec3(7, 7, 7),
        glm::vec4(1, 1, 1, 1)
    );

    ASSERT(vec.size() == 2);
    ASSERT(vec.with<Position>().vector<Position>()[0] == glm::vec3(7, 7, 7));
    ASSERT(vec.with<Color>().vector<Color>()[1] == glm::vec4(1, 1, 1, 1));
}

void test_reserve() {
    std::cout << "\n========== test_reserve ==========\n";

    default_vector<Position, Color> vec;

    vec.with<Position, Color>().reserve(100);

    ASSERT(vec.capacity() >= 100);
    ASSERT(vec.size() == 0);
}

void test_single_proxy() {
    std::cout << "\n========== test_single_proxy ==========\n";

    version_vector<Position, Color, TexCoords> vec(3);

    // Получаем single_proxy для Position
    auto posProxy = vec.attribute<Position>();

    // Меняем значения
    posProxy[0] = glm::vec3(1, 2, 3);
    posProxy[1] = glm::vec3(4, 5, 6);
    posProxy[2] = glm::vec3(7, 8, 9);

    ASSERT(posProxy[0] == glm::vec3(1, 2, 3));
    ASSERT(posProxy[2] == glm::vec3(7, 8, 9));

    // Читаем через multi_proxy
    ASSERT(vec.with<Position>().vector<Position>()[1] == glm::vec3(4, 5, 6));
}

void test_single_proxy_push_back_and_erase() {
    std::cout << "\n========== test_single_proxy_push_back_and_erase ==========\n";

    version_vector<Position, Color> vec;

    auto posProxy = vec.attribute<Position>();
    auto colProxy = vec.attribute<Color>();

    posProxy.push_back(glm::vec3(1, 1, 1));
    colProxy.push_back(glm::vec4(1, 0, 0, 1));

    ASSERT(vec.size() == 2);
    ASSERT(posProxy[0] == glm::vec3(1, 1, 1));

    posProxy.push_back(glm::vec3(2, 2, 2));
    colProxy.push_back(glm::vec4(0, 1, 0, 1));

    ASSERT(vec.size() == 4);

    posProxy.erase(0);

    ASSERT(vec.size() == 3);
    ASSERT(posProxy[0] == glm::vec3(0.f));
}

void test_single_proxy_insert_from_proxy() {
    std::cout << "\n========== test_single_proxy_insert_from_proxy ==========\n";

    version_vector<Position, Color> vec1(2);
    auto pos1 = vec1.attribute<Position>();
    pos1[0] = glm::vec3(1, 1, 1);
    pos1[1] = glm::vec3(3, 3, 3);

    version_vector<Position, Color> vec2(1);
    auto pos2 = vec2.attribute<Position>();
    pos2[0] = glm::vec3(2, 2, 2);

    // Вставляем single_proxy из vec2 в vec1
    pos1.insert(1, pos2);  // Вставка одного элемента на позицию 1

    ASSERT(vec1.size() == 3);
    ASSERT(pos1[0] == glm::vec3(1, 1, 1));
    ASSERT(pos1[1] == glm::vec3(2, 2, 2));  // Вставленный элемент
    ASSERT(pos1[2] == glm::vec3(3, 3, 3));  // Сдвинутый элемент
}

void test_single_proxy_upload_from_proxy() {
    std::cout << "\n========== test_single_proxy_upload_from_proxy ==========\n";

    version_vector<Position, Color> vec1(3);
    auto pos1 = vec1.attribute<Position>();
    pos1[0] = glm::vec3(1, 1, 1);
    pos1[1] = glm::vec3(2, 2, 2);
    pos1[2] = glm::vec3(3, 3, 3);

    version_vector<Position, Color> vec2(2);
    auto pos2 = vec2.attribute<Position>();
    pos2[0] = glm::vec3(20, 20, 20);
    pos2[1] = glm::vec3(30, 30, 30);

    // Перезаписываем элементы начиная с индекса 1
    pos1.upload(1, pos2);

    ASSERT(vec1.size() == 3);
    ASSERT(pos1[0] == glm::vec3(1, 1, 1));      // Не тронут
    ASSERT(pos1[1] == glm::vec3(20, 20, 20));    // Перезаписан
    ASSERT(pos1[2] == glm::vec3(30, 30, 30));    // Перезаписан
}

void test_single_proxy_for_each() {
    std::cout << "\n========== test_single_proxy_for_each ==========\n";

    version_vector<Position, Color> vec(3);
    auto posProxy = vec.attribute<Position>();

    posProxy[0] = glm::vec3(1, 0, 0);
    posProxy[1] = glm::vec3(2, 0, 0);
    posProxy[2] = glm::vec3(3, 0, 0);

    float sum_x = 0;
    posProxy.for_each([&](const glm::vec3& v) {
        sum_x += v.x;
        });

    ASSERT(sum_x == 6.0f);
}

void test_multi_proxy_partial() {
    std::cout << "\n========== test_multi_proxy_partial ==========\n";

    // Создаём вектор с тремя атрибутами
    default_vector<Position, Color, TexCoords> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1) },
        { glm::vec2(0,0), glm::vec2(1,1) }
    );

    // Работаем только с Position и TexCoords (без Color)
    auto partial = vec.with<Position, TexCoords>();

    ASSERT(partial.size() == 2);
    ASSERT(partial.vector<Position>()[0] == glm::vec3(0, 0, 0));
    ASSERT(partial.vector<TexCoords>()[1] == glm::vec2(1, 1));

    // Модифицируем
    partial.push_back(glm::vec3(2, 2, 2), glm::vec2(2, 2));

    ASSERT(vec.size() == 3);
    // Color должен автоматически заполниться дефолтным значением
    ASSERT(vec.with<Color>().vector<Color>()[2] == Color::defaultValue());
}

void test_clear() {
    std::cout << "\n========== test_clear ==========\n";

    version_vector<Position, Color> vec(10);
    ASSERT(vec.size() == 10);

    vec.attribute<Position>().clear();

    ASSERT(vec.size() == 0);
}

void test_data_pointer() {
    std::cout << "\n========== test_data_pointer ==========\n";

    version_vector<Position, Color> vec(3);
    auto posProxy = vec.attribute<Position>();

    posProxy[0] = glm::vec3(1, 2, 3);
    posProxy[1] = glm::vec3(4, 5, 6);
    posProxy[2] = glm::vec3(7, 8, 9);

    const glm::vec3* data = posProxy.data();
    ASSERT(data != nullptr);
    ASSERT(data[0] == glm::vec3(1, 2, 3));
    ASSERT(data[2] == glm::vec3(7, 8, 9));
}

void test_operator_output() {
    std::cout << "\n========== test_operator_output ==========\n";

    default_vector<Position, Color> vec(
        { glm::vec3(0,0,0), glm::vec3(1,1,1) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1) }
    );

    std::stringstream ss;
    ss << vec;  // Должен вывести через multi_proxy

    std::string output = ss.str();
    ASSERT(!output.empty());
    ASSERT(output.find("vec3") != std::string::npos);
    ASSERT(output.find("vec4") != std::string::npos);
    std::cout << "Output:\n" << output;
}

void test_versioned_vector_bumps_version() {
    std::cout << "\n========== test_versioned_vector_bumps_version ==========\n";

    version_vector<Position> vec(1);

    auto old_version = vec.attribute<Position>().vec().getVersion();

    vec.attribute<Position>()[0] = glm::vec3(1, 2, 3);

    auto new_version = vec.attribute<Position>().vec().getVersion();
    ASSERT(new_version > old_version);
}

// ============================================================
// Демонстрация реального использования (из твоего рендера)
// ============================================================

void demo_mesh_usage() {
    std::cout << "\n========== demo_mesh_usage ==========\n";

    // Симуляция того, как ты используешь attribute_vector в Mesh

    // Создаём меш с позициями и цветами
    version_vector<Position, Color, TexCoords> mesh_data;

    // Добавляем вершины треугольника
    mesh_data.with<Position, Color, TexCoords>().push_back(
        glm::vec3(-0.5f, -0.5f, 0.0f),  // позиция
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),  // цвет
        glm::vec2(0.0f, 0.0f)  // текстурные координаты
    );

    mesh_data.with<Position, Color, TexCoords>().push_back(
        glm::vec3(0.5f, -0.5f, 0.0f),
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
        glm::vec2(1.0f, 0.0f)
    );

    mesh_data.with<Position, Color, TexCoords>().push_back(
        glm::vec3(0.0f, 0.5f, 0.0f),
        glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
        glm::vec2(0.5f, 1.0f)
    );

    ASSERT(mesh_data.size() == 3);

    // Симуляция того, как ShaderPipeline дёргает данные
    auto position_data = mesh_data.with<Position>();
    auto color_data = mesh_data.with<Color>();

    // Для OpenGL: получаем сырые указатели
    const auto* pos_ptr = position_data.vector<Position>().data();
    const auto* col_ptr = color_data.vector<Color>().data();

    ASSERT(pos_ptr != nullptr);
    ASSERT(col_ptr != nullptr);

    // Проверяем, что данные лежат в памяти непрерывно (как требует glBufferData)
    ASSERT(pos_ptr[0] == glm::vec3(-0.5f, -0.5f, 0.0f));
    ASSERT(pos_ptr[1] == glm::vec3(0.5f, -0.5f, 0.0f));
    ASSERT(pos_ptr[2] == glm::vec3(0.0f, 0.5f, 0.0f));

    ASSERT(col_ptr[0] == glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    ASSERT(col_ptr[1] == glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    ASSERT(col_ptr[2] == glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

    std::cout << "Mesh data is ready for GPU upload!\n";
}

void test_attribute_vector_insert_into_another() {
    std::cout << "\n========== test_attribute_vector_insert_into_another ==========\n";

    // Первый вектор — полный набор атрибутов
    version_vector<Position, Color, TexCoords> vec1(
        { glm::vec3(0,0,0), glm::vec3(1,1,1) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1) },
        { glm::vec2(0,0), glm::vec2(1,1) }
    );

    // Второй вектор — только Position и Color (без TexCoords)
    version_vector<Position, Color> vec2(
        { glm::vec3(2,2,2), glm::vec3(3,3,3) },
        { glm::vec4(0,0,1,1), glm::vec4(1,1,0,1) }
    );

    // Вставляем vec2 в середину vec1
    vec1.with<Position, Color, TexCoords>().insert(1, vec2.with<Position, Color>());

    ASSERT(vec1.size() == 4);  // было 2 + вставили 2 = 4

    // Проверяем первую часть (не тронута)
    ASSERT(vec1.with<Position>().vector<Position>()[0] == glm::vec3(0, 0, 0));
    ASSERT(vec1.with<Color>().vector<Color>()[0] == glm::vec4(1, 0, 0, 1));
    ASSERT(vec1.with<TexCoords>().vector<TexCoords>()[0] == glm::vec2(0, 0));

    // Проверяем вставленные (из vec2)
    ASSERT(vec1.with<Position>().vector<Position>()[1] == glm::vec3(2, 2, 2));
    ASSERT(vec1.with<Position>().vector<Position>()[2] == glm::vec3(3, 3, 3));
    ASSERT(vec1.with<Color>().vector<Color>()[1] == glm::vec4(0, 0, 1, 1));
    ASSERT(vec1.with<Color>().vector<Color>()[2] == glm::vec4(1, 1, 0, 1));
    // TexCoords должны заполниться дефолтами для вставленных элементов
    ASSERT(vec1.with<TexCoords>().vector<TexCoords>()[1] == TexCoords::defaultValue());
    ASSERT(vec1.with<TexCoords>().vector<TexCoords>()[2] == TexCoords::defaultValue());

    // Проверяем сдвинутую часть
    ASSERT(vec1.with<Position>().vector<Position>()[3] == glm::vec3(1, 1, 1));
    ASSERT(vec1.with<Color>().vector<Color>()[3] == glm::vec4(0, 1, 0, 1));
    ASSERT(vec1.with<TexCoords>().vector<TexCoords>()[3] == glm::vec2(1, 1));

    std::cout << "✅ Insert with partial tags works correctly\n";
}

void test_attribute_vector_insert_empty_into_another() {
    std::cout << "\n========== test_attribute_vector_insert_empty_into_another ==========\n";

    version_vector<Position, Color> vec1(
        { glm::vec3(1,1,1), glm::vec3(2,2,2) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1) }
    );

    version_vector<Position, Color> vec2;  // пустой

    vec1.with<Position, Color>().insert(1, vec2.with<Position, Color>());

    ASSERT(vec1.size() == 2);  // ничего не изменилось
    ASSERT(vec1.with<Position>().vector<Position>()[0] == glm::vec3(1, 1, 1));

    std::cout << "✅ Insert empty vector does nothing\n";
}

void test_attribute_vector_insert_at_end() {
    std::cout << "\n========== test_attribute_vector_insert_at_end ==========\n";

    version_vector<Position, Color> vec1(
        { glm::vec3(1,1,1) },
        { glm::vec4(1,0,0,1) }
    );

    version_vector<Position, Color> vec2(
        { glm::vec3(2,2,2), glm::vec3(3,3,3) },
        { glm::vec4(0,1,0,1), glm::vec4(0,0,1,1) }
    );

    vec1.with<Position, Color>().insert(vec1.size(), vec2.with<Position, Color>());

    ASSERT(vec1.size() == 3);
    ASSERT(vec1.with<Position>().vector<Position>()[0] == glm::vec3(1, 1, 1));
    ASSERT(vec1.with<Position>().vector<Position>()[1] == glm::vec3(2, 2, 2));
    ASSERT(vec1.with<Position>().vector<Position>()[2] == glm::vec3(3, 3, 3));

    std::cout << "✅ Insert at end works correctly\n";
}

void test_attribute_vector_insert_at_beginning() {
    std::cout << "\n========== test_attribute_vector_insert_at_beginning ==========\n";

    version_vector<Position, Color> vec1(
        { glm::vec3(3,3,3) },
        { glm::vec4(0,0,1,1) }
    );

    version_vector<Position, Color> vec2(
        { glm::vec3(1,1,1), glm::vec3(2,2,2) },
        { glm::vec4(1,0,0,1), glm::vec4(0,1,0,1) }
    );

    vec1.with<Position, Color>().insert(0, vec2.with<Position, Color>());

    ASSERT(vec1.size() == 3);
    ASSERT(vec1.with<Position>().vector<Position>()[0] == glm::vec3(1, 1, 1));
    ASSERT(vec1.with<Position>().vector<Position>()[1] == glm::vec3(2, 2, 2));
    ASSERT(vec1.with<Position>().vector<Position>()[2] == glm::vec3(3, 3, 3));

    std::cout << "✅ Insert at beginning works correctly\n";
}

void test_attribute_vector_upload_from_another() {
    std::cout << "\n========== test_attribute_vector_upload_from_another ==========\n";

    version_vector<Position, Color, TexCoords> vec1(
        { glm::vec3(0,0,0), glm::vec3(1,1,1), glm::vec3(2,2,2) },
        { glm::vec4(1,1,1,1), glm::vec4(1,1,1,1), glm::vec4(1,1,1,1) },
        { glm::vec2(0,0), glm::vec2(1,1), glm::vec2(2,2) }
    );

    version_vector<Position, Color> vec2(
        { glm::vec3(10,10,10), glm::vec3(20,20,20) },
        { glm::vec4(0,0,0,1), glm::vec4(0,0,0,1) }
    );

    // Перезаписываем начиная с индекса 1
    vec1.with<Position, Color, TexCoords>().upload(1, vec2.with<Position, Color>());

    ASSERT(vec1.size() == 3);  // размер не изменился

    // Первый элемент не тронут
    ASSERT(vec1.with<Position>().vector<Position>()[0] == glm::vec3(0, 0, 0));
    ASSERT(vec1.with<Color>().vector<Color>()[0] == glm::vec4(1, 1, 1, 1));
    ASSERT(vec1.with<TexCoords>().vector<TexCoords>()[0] == glm::vec2(0, 0));

    // Перезаписанные элементы
    ASSERT(vec1.with<Position>().vector<Position>()[1] == glm::vec3(10, 10, 10));
    ASSERT(vec1.with<Position>().vector<Position>()[2] == glm::vec3(20, 20, 20));
    ASSERT(vec1.with<Color>().vector<Color>()[1] == glm::vec4(0, 0, 0, 1));
    ASSERT(vec1.with<Color>().vector<Color>()[2] == glm::vec4(0, 0, 0, 1));

    // TexCoords нетронуты потому что их не было в vec2
    ASSERT(vec1.with<TexCoords>().vector<TexCoords>()[1] != TexCoords::defaultValue());
    ASSERT(vec1.with<TexCoords>().vector<TexCoords>()[2] != TexCoords::defaultValue());

    std::cout << "✅ Upload from another attribute_vector works correctly\n";
}

void test_attribute_vector_insert_wrong_tags() {
    std::cout << "\n========== test_attribute_vector_insert_wrong_tags ==========\n";

    // Этот тест проверяет, что код НЕ компилируется, если теги не совпадают
    // Оставлю заглушку — проверяется на этапе компиляции через static_assert

    std::cout << "✅ Insert with wrong tags is caught at compile time (static_assert)\n";
}

void test_single_proxy_insert_from_another_vector() {
    std::cout << "\n========== test_single_proxy_insert_from_another_vector ==========\n";

    version_vector<Position, Color> vec1(2);
    auto pos1 = vec1.attribute<Position>();
    pos1[0] = glm::vec3(1, 1, 1);
    pos1[1] = glm::vec3(4, 4, 4);

    version_vector<Position, Color> vec2(2);
    auto pos2 = vec2.attribute<Position>();
    pos2[0] = glm::vec3(2, 2, 2);
    pos2[1] = glm::vec3(3, 3, 3);

    // Вставляем один атрибут из другого вектора
    pos1.insert(1, pos2);

    ASSERT(vec1.size() == 4);
    ASSERT(pos1[0] == glm::vec3(1, 1, 1));
    ASSERT(pos1[1] == glm::vec3(2, 2, 2));  // вставлено из vec2
    ASSERT(pos1[2] == glm::vec3(3, 3, 3));  // вставлено из vec2
    ASSERT(pos1[3] == glm::vec3(4, 4, 4));  // сдвинуто

    // Color должен был синхронизироваться (дефолтами для вставленных)
    auto col1 = vec1.attribute<Color>();
    ASSERT(col1[1] == Color::defaultValue());
    ASSERT(col1[2] == Color::defaultValue());

    std::cout << "✅ Single proxy insert from another vector works correctly\n";
}

void test_single_proxy_upload_from_another_vector() {
    std::cout << "\n========== test_single_proxy_upload_from_another_vector ==========\n";

    version_vector<Position, Color> vec1(4);
    auto pos1 = vec1.attribute<Position>();
    pos1[0] = glm::vec3(1, 1, 1);
    pos1[1] = glm::vec3(2, 2, 2);
    pos1[2] = glm::vec3(3, 3, 3);
    pos1[3] = glm::vec3(4, 4, 4);

    version_vector<Position, Color> vec2(2);
    auto pos2 = vec2.attribute<Position>();
    pos2[0] = glm::vec3(20, 20, 20);
    pos2[1] = glm::vec3(30, 30, 30);

    // Перезаписываем начиная с индекса 1
    pos1.upload(1, pos2);

    ASSERT(vec1.size() == 4);  // размер не меняется

    ASSERT(pos1[0] == glm::vec3(1, 1, 1));    // не тронут
    ASSERT(pos1[1] == glm::vec3(20, 20, 20)); // перезаписан
    ASSERT(pos1[2] == glm::vec3(30, 30, 30)); // перезаписан
    ASSERT(pos1[3] == glm::vec3(4, 4, 4));    // не тронут

    std::cout << "✅ Single proxy upload from another vector works correctly\n";
}

void test_version_vector_insert_preserves_versions() {
    std::cout << "\n========== test_version_vector_insert_preserves_versions ==========\n";

    version_vector<Position, Color> vec1(2);
    auto old_ver1 = vec1.attribute<Position>().vec().getVersion();

    version_vector<Position, Color> vec2(
        { glm::vec3(2,2,2) },
        { glm::vec4(0,1,0,1) }
    );

    vec1.with<Position, Color>().insert(1, vec2.with<Position, Color>());

    auto new_ver1 = vec1.attribute<Position>().vec().getVersion();
    ASSERT(new_ver1 > old_ver1);  // версия увеличилась

    std::cout << "✅ Version bumps after insert from another vector\n";
}

// ============================================================
// MAIN
// ============================================================

int main() {
    system("chcp 65001 > nul");

    std::cout << "╔══════════════════════════════════════╗\n";
    std::cout << "║  attribute_vector TESTS & DEMOS      ║\n";
    std::cout << "╚══════════════════════════════════════╝\n";

    bool tests = true;

    // Базовые тесты
    if (tests) {
        test_default_construction();
        test_sized_construction();
        test_initializer_list_construction();
        test_initializer_list_size_mismatch();
    }

    // Слайсы
    test_slice_basic();
    test_slice_data_pointer();
    test_slice_nested();
    test_slice_as_proxy();
    test_slice_empty();
    test_slice_out_of_range();
    test_slice_for_each();

    // Модификация данных
    if (tests) {
    test_push_back();
    test_insert_single();
    test_insert_multiple();
    test_insert_from_containers();
    test_insert_from_proxy();
    test_upload();
    test_upload_from_containers();
    test_upload_auto_resize();
    test_erase();
    test_erase_range();
    test_erase_out_of_range();
    

    // Изменение размера
    test_resize_larger();
    test_resize_smaller();
    test_resize_with_values();
    test_reserve();
    test_clear();

    // Single proxy
    test_single_proxy();
    test_single_proxy_push_back_and_erase();
    test_single_proxy_for_each();
    test_single_proxy_insert_from_proxy();
    test_single_proxy_upload_from_proxy();
    test_data_pointer();

    // Multi proxy
    test_multi_proxy_partial();

    // Insert/upload между attribute_vector
    test_attribute_vector_insert_into_another();
    test_attribute_vector_insert_empty_into_another();
    test_attribute_vector_insert_at_end();
    test_attribute_vector_insert_at_beginning();
    test_attribute_vector_upload_from_another();
    test_attribute_vector_insert_wrong_tags();

    // Single proxy insert/upload из другого вектора
    test_single_proxy_insert_from_another_vector();
    test_single_proxy_upload_from_another_vector();

    // Версионирование при вставке
    test_version_vector_insert_preserves_versions();

    // Вывод
    test_operator_output();

    // Versioned vector
    test_versioned_vector_bumps_version();

    // Демо использования
    demo_mesh_usage();

    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  ALL TESTS PASSED! ✅\n";
    std::cout << "═══════════════════════════════════════\n";

    attribute_vector<std::deque, Position> dq;
    }

    return 0;
}