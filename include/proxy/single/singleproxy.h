#pragma once

#include <attribute_vector/attribute_vector.h>
#include <proxy/multi/multi_proxy.h>

template<IsAttributeVector AttributeVectorT, bool IsConst, typename Tag>
class single_proxy : private multi_proxy<AttributeVectorT, IsConst, Tag> {
    using Base = multi_proxy<AttributeVectorT, IsConst, Tag>;

    // Открываем допступ к родительским методам и полям
    template<typename VecTag>
    using vec_type = typename Base::template vec_type<VecTag>;
    using Base::data_;
    using Base::call;

public:
    using Base::tags;

    using Base::size;
    using Base::owner_tags;
    using Base::vector;
    using Base::Base;
    using Base::insert_list;
    using Base::insert;
    using Base::erase;
    using Base::reserve;
    using Base::resize;
    using Base::upload;
    using Base::upload_list;
    using Base::push_back;
    using Base::pop_back;

    auto begin() {
        return this->mutable_vector<Tag>().begin();
    }
    auto end() {
        return this->mutable_vector<Tag>().end();
    }

    // Доступ к элементу
    decltype(auto) operator[](size_t i) {
        return this->mutable_vector<Tag>()[i];
    }
    const auto& operator[](size_t i) const {
        return this->vector<Tag>()[i];
    }

    // Прямой доступ к данным
    const typename Tag::type* data() const {
        return this->vector<Tag>().data();
    }

    // Доступ к вектору
    const auto& vec() const {
        return this->vector<Tag>();
    }

    // Очистка
    void clear() requires (!IsConst) {
        this->resize(0);
    }

    // Итерация
    template<typename F>
    void for_each(F&& func) const {
        for (size_t i = 0; i < this->size(); ++i) {
            func((*this)[i]);
        }
    }

    template<typename F>
    void for_each(F&& func) requires (!IsConst) {
        for (size_t i = 0; i < this->size(); ++i) {
            func((*this)[i]);
        }
    }

    // Загрузка данных (upload) – обёртки над методами MultiProxy
    template<typename Container>
    void upload_containers(size_t where, const Container& container) requires (!IsConst) {
        static_assert(std::is_same_v<typename Container::value_type, typename Tag::type>,
            "Container value type does not match tag type");
        this->upload_containers(where, container);
    }

    void insert_containers(size_t where, const std::vector<typename Tag::type>& container) requires (!IsConst) {
        if (where > this->size()) throw std::out_of_range("Insert position out of range");
        this->insert_containers(where, container);
    }
};