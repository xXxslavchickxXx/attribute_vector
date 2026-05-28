#pragma once

#include <type_traits/type_traits.h>
#include <tuple>

// Концепт на соответствие форме типа атрибут вектора
template<typename T>
concept IsAttributeVector = requires {
    typename T::data_type;
    typename T::tags;
    typename T::template vec_type<int>;
};

template<IsAttributeVector AttributeVectorT, bool IsConst, typename... SelectedTags>
class base_proxy {
protected:
/// Алиасы
    // Шаблонный алиас для выведения типа вектора в тупле
    template<typename Tag>
    using vec_type = typename AttributeVectorT::template vec_type<typename Tag::type>;
    // Тип тупла с данными
    using DataType = typename AttributeVectorT::data_type;
    using PointerType = std::conditional_t<IsConst, const DataType*, DataType*>;
    // Теги
public:
    using tags = std::tuple<SelectedTags...>;
    using owner_tags = AttributeVectorT::tags;

/// Данные класса
protected:
    PointerType data_;

/// Методы класса
public:
	base_proxy(PointerType data);
    base_proxy(DataType& data);
    base_proxy(const DataType& data);

    template<typename Tag>
    const vec_type<Tag>& vector() const;

    size_t size() const;
    size_t capacity() const;
    bool empty() const;

    // Получение изменяемого массива по тегу
    template<typename Tag>
    vec_type<Tag>& mutable_vector();

protected:
    // Получения индекса тега для доступа в тупл
    template<typename Tag>
    constexpr size_t get_tag_index() const;

    // Удобный метод для вызова лямбд без аргв по шаблону
    template<typename Tag, typename F>
    constexpr void call(F&& f) {
        f.template operator() < Tag > ();
    }

    friend std::ostream& operator<<(std::ostream& os, const base_proxy& proxy) {
        ((os << proxy.vector<SelectedTags>() << '\n'), ...);
        return os;
    }

};

#include "base.inl"