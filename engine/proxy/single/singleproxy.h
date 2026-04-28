#pragma once

#include "../../attribute_vector/attribute_vector.h"

namespace engine::data {

    template<template<typename...> typename Vec, typename... Tags>
    template<bool IsConst, typename Tag>
    class attribute_vector<Vec, Tags...>::single_proxy {
        using TupleType = std::tuple<Vec<typename Tags::type>...>;
        using PointerType = std::conditional_t<IsConst, const TupleType*, TupleType*>;

        multi_proxy<IsConst, Tag> _proxy;

    public:
        using is_single_proxy_tag = void;

        single_proxy(TupleType& data) : _proxy(data) {}

        template<typename T>
        void insert(size_t where, const T& otherSP) requires (!IsConst && is_single_proxy_v<T>) {
            _proxy.insert(where, otherSP._proxy);
        }

        template<typename T>
        void upload(size_t where, const T& otherSP) requires (!IsConst && is_single_proxy_v<T>) {
            _proxy.upload(where, otherSP._proxy);
        }

        // Доступ к элементу
        decltype(auto) operator[](size_t i) {
            return _proxy.template mutable_vector<Tag>()[i];
        }
        const auto& operator[](size_t i) const {
            return _proxy.template vector<Tag>()[i];
        }

        // Добавление элемента в конец
        void push_back(const typename Tag::type& value) requires (!IsConst) {
            _proxy.insert(size(), 1, value); // используем вставку в конец
        }

        // Размер и ёмкость
        size_t size() const { return _proxy.size(); }
        size_t capacity() const { return _proxy.capacity(); }

        // Прямой доступ к данным (для GPU)
        const typename Tag::type* data() const {
            return _proxy.template vector<Tag>().data();
        }

        // Доступ к вектору (если нужен)
        const Vec<typename Tag::type>& vec() const {
            return _proxy.template vector<Tag>();
        }

        // Изменение размера
        void resize(size_t new_size) requires (!IsConst) {
            _proxy.resize(new_size);
        }
        void resize(size_t new_size, const typename Tag::type& value) requires (!IsConst) {
            _proxy.template mutable_vector<Tag>().resize(new_size, value);
            // Синхронизация остальных тегов (дефолтами)
            auto sync = [&]<typename OtherTag>() {
                _proxy.template mutable_vector<OtherTag>().resize(new_size, OtherTag::defaultValue());
            };
            _proxy.execute_for_other(sync);
        }

        // Резервирование
        void reserve(size_t new_cap) requires (!IsConst) {
            _proxy.reserve(new_cap);
        }

        // Удаление
        void erase(size_t pos) requires (!IsConst) {
            _proxy.erase(pos);
        }
        void erase(size_t pos, size_t n) requires (!IsConst) {
            _proxy.erase(pos, n);
        }

        // Очистка
        void clear() requires (!IsConst) {
            _proxy.resize(0);
        }

        // Итерация
        template<typename F>
        void for_each(F&& func) const {
            for (size_t i = 0; i < size(); ++i) {
                func((*this)[i]);
            }
        }

        template<typename F>
        void for_each(F&& func) requires (!IsConst) {
            for (size_t i = 0; i < size(); ++i) {
                func((*this)[i]);
            }
        }

        // Загрузка данных (upload) – обёртки над методами MultiProxy
        template<typename Container>
        void upload_containers(size_t where, const Container& container) requires (!IsConst) {
            static_assert(std::is_same_v<typename Container::value_type, typename Tag::type>,
                "Container value type does not match tag type");
            _proxy.upload_containers(where, container);
        }

        void upload_list(size_t where, std::initializer_list<typename Tag::type> list) requires (!IsConst) {
            _proxy.upload_list(where, list);
        }

        void upload(size_t where, const typename Tag::type& value) requires (!IsConst) {
            _proxy.upload(where, value);
        }

        // Вставка
        void insert(size_t where, const typename Tag::type& value) requires (!IsConst) {
            if (where > size()) throw std::out_of_range("Insert position out of range");
            _proxy.insert(where, 1, value);
        }

        void insert(size_t where, size_t n, const typename Tag::type& value) requires (!IsConst) {
            if (where > size()) throw std::out_of_range("Insert position out of range");
            _proxy.insert(where, n, value);
        }

        void insert_containers(size_t where, const std::vector<typename Tag::type>& container) requires (!IsConst) {
            if (where > size()) throw std::out_of_range("Insert position out of range");
            _proxy.insert_containers(where, container);
        }
    };

}