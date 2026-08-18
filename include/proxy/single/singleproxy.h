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
    using typename Base::tags;
    using typename Base::owner_tags;

    using Base::size;
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
    using Base::empty;
    using Base::clear;

    auto begin() {
        return this->template mutable_vector<Tag>().begin();
    }
    auto end() {
        return this->template mutable_vector<Tag>().end();
    }

    typename Tag::type back() const {
        return vec().back();
    }
    typename Tag::type front() const {
        return vec().front();
    }

    // Доступ к элементу
    decltype(auto) operator[](size_t i) {
        return this->template mutable_vector<Tag>()[i];
    }
    const auto& operator[](size_t i) const {
        return this->template vector<Tag>()[i];
    }

    const typename Tag::type* data() const {
        return this->template vector<Tag>().data();
    }

    const auto& vec() const {
        return this->template vector<Tag>();
    }

    void clear() requires (!IsConst) {
        this->resize(0);
    }

    // Итерация с каждым элементом
    template<typename F>
    void for_each(F&& func) const {
        auto& vec = this->template mutable_vector<Tag>();
        for (auto& item : vec) {
            func(item);
        }
    }

    template<typename F>
    void for_each(F&& func) requires (!IsConst) {
        auto& vec = this->template mutable_vector<Tag>();
        for (auto& item : vec) {
            func(item);
        }
    }

    // Загрузка данных 'upload' – обёртки над методами MultiProxy
    template<typename Container>
    void upload_containers(size_t where, Container&& container) requires (!IsConst) {
        static_assert(std::is_same_v<typename std::decay_t<Container>::value_type,
                                     typename Tag::type>,
            "Container value type does not match tag type");
        Base::upload_containers(where, std::forward<Container>(container));
    }

    template<typename Container>
    void insert_containers(size_t where, Container&& container) requires (!IsConst) {
        if (where > this->size()) throw std::out_of_range("Insert position out of range");
        Base::insert_containers(where, std::forward<Container>(container));
    }
};