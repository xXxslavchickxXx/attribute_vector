# Attribute Vector (`attribute_vector`)

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Header-Only](https://img.shields.io/badge/library-header--only-brightgreen.svg)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)

**`attribute_vector`** — это производительный C++17 header-only контейнер для удобного хранения и обработки разнородных атрибутов объектов.

Библиотека физически хранит данные в формате **SoA** (Structure of Arrays) — каждый атрибут находится в отдельном последовательном массиве, что критически важно для эффективного использования кэша процессора. При этом работа с контейнером происходит через интуитивный интерфейс, похожий на привычный **AoS** (Array of Structures).

## Пример использования

```cpp
#include <attribute_vector/attribute_vector.h>
#include <test_tags/tags.h>

int main() {
    using namespace test;

    // Создаем вектор с атрибутами Position и Color
    auto points = default_vector<Position, Color>();

    // Добавляем точку с позицией и цветом
    points.with<Position, Color>().push_back(
        glm::vec3(1.0f, 2.0f, 3.0f),
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
    );

    // Добавляем точку только с позицией (цвет — дефолтный)
    points.attribute<Position>().push_back(
        glm::vec3(4.0f, 5.0f, 6.0f)
    );

    // Читаем данные
    auto pos = points.attribute<Position>();
    std::cout << "Position[0]: " << pos[0].x << ", " << pos[0].y << ", " << pos[0].z << "\n";

    return 0;
}
```

## Ключевые особенности

- **Struct of arrays:** Максимальная локальность данных по кэшу при итерации по конкретным атрибутам.
- **Удобный интерфейс:** Работа с несколькими параллельными векторами как с единой сущностью.
- **Header-Only:** Никакой сложной сборки — просто подключите заголовочные файлы.
- **Безопасность типов:** Проверки во время компиляции на основе шаблонов C++17.
- **Гибкость:** Автоматическое заполнение дефолтными значениями при частичном добавлении атрибутов.

## Требования

- Компилятор MSVC с поддержкой **C++17** или выше (пока что не кросскомпилируемый, другие компиляторы более строгие к синтаксису шаблонов).
- **GLM** *(опционально)* — требуется только при использовании тестовых тегов из `test_tags/tags.h`.

---

# Установка

Библиотека является **header-only** и не требует предварительной сборки.

### Способ 1. FetchContent (Рекомендуемый для CMake 3.11+)

Добавьте следующий блок в ваш `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    attribute_vector
    GIT_REPOSITORY https://github.com/xXxslavchickxXx/attribute_vector.git
    GIT_TAG main
)
FetchContent_MakeAvailable(attribute_vector)

target_link_libraries(ваш_проект PRIVATE attribute_vector)
```

### Способ 2: Использовать проект как подмодуль

1. Скопируйте папку проект в директорию вашего проекта.
2. Добавьте путь к ней в настройки компилятора:

```cmake
# Скопируйте папку с библиотекой в свой проект и добавьте:
add_subdirectory(path/to/attribute_vector)
target_link_libraries(ваш_проект PRIVATE attribute_vector)
```

### Способ 3: Скопировать заголовочные файлы

1. Скопируйте папку include/ из репозитория в свой проект.
2. Добавьте путь к include/ в настройки компилятора:

CMake:

```cmake
target_include_directories(ваш_проект PRIVATE путь/к/attribute_vector/include)
```

GCC/Clang:

```bash
g++ -I путь/к/attribute_vector/include ваш_мейн.cpp
```

# Структура репозитория

```
attribute_vector/
├── include/
│   └── attribute_vector/
│   |   ├── attribute_vector.h
│   |   ├── attribute_vector.inl
│   └── ... (остальные папки с заголовками)
├── test_tags/
│   └── tags.h              # Примеры тегов (Position, Color, TexCoords)
├── tests/test.cpp          # Пользоательские-тесты
├── README.md
└── LICENSE
└── CMakeLists.txt
```

## Контрибьюция
Если вы нашли баг или хотите предложить улучшение — открывайте Issue или Pull Request. Любая помощь приветствуется!

## Контакты
- GitHub: xXxslavchickxXx

- Email: slaverint@gmail.com

Если проект оказался полезным — поставьте звезду на GitHub!
