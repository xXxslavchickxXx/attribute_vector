#pragma once

#include <vector>
#include <iostream>
#include <limits>

namespace versions {
    class Versioned {
        unsigned int version = 0;
    public:
        void bumpVersion() { version++; }
        unsigned int getVersion() const { return version; }
    };
}

template<typename T, typename F>
class Field {
    F _func;
    T& _value;

public:
    Field(F&& func, T& value) : _func(std::forward<F>(func)), _value(value) {}

    Field& operator=(const T& new_val) {
        _value = new_val;
        _func();
        return *this;
    }
    Field& operator=(T&& new_val) {
        _value = std::move(new_val);
        _func();
        return *this;
    }

    operator const T& () const { return _value; }
    bool operator==(const T& data) { return data == _value; }
    bool operator!=(const T& data) { return data != _value; }

    // Доступ к членам, если T — структура
    const auto* operator->() const { return &_value; }
};

template<typename T>
class versionedVector {
private:
    std::vector<T> rawVector;
    versions::Versioned version;

    // Dirty range tracking
    size_t dirty_begin = std::numeric_limits<size_t>::max();
    size_t dirty_end = 0;

    // Core dirty tracking methods
    void mark_dirty_index(size_t index) {
        if (index < dirty_begin) dirty_begin = index;
        if (index + 1 > dirty_end) dirty_end = index + 1;
        version.bumpVersion();
    }

    void mark_dirty_range(size_t begin, size_t end) {
        if (begin < dirty_begin) dirty_begin = begin;
        if (end > dirty_end) dirty_end = end;
        version.bumpVersion();
    }

    void mark_all_dirty() {
        if (!rawVector.empty()) {
            dirty_begin = 0;
            dirty_end = rawVector.size();
        }
        version.bumpVersion();
    }

public:
    using value_type = std::vector<T>::value_type;

    struct ranges {
        size_t begin;
        size_t end;

        bool empty() const { return begin >= end; }
        size_t size() const { return end - begin; }
    };

    // Dirty range API
    ranges get_dirty_range() const {
        if (dirty_begin == std::numeric_limits<size_t>::max()) {
            return { 0, 0 };
        }
        return { dirty_begin, dirty_end };
    }

    void clear_dirty_range() {
        dirty_begin = std::numeric_limits<size_t>::max();
        dirty_end = 0;
    }

    bool has_dirty_range() const {
        return dirty_begin != std::numeric_limits<size_t>::max() && dirty_begin < dirty_end;
    }

    auto operator[](size_t i) {
        auto mark = [this, i]() { mark_dirty_index(i); };
        return Field<T, decltype(mark)>(std::move(mark), rawVector[i]);
    }

    const T& operator[](size_t i) const {
        return rawVector[i];
    }

    operator const std::vector<T>& () const { return rawVector; }

    versionedVector() = default;

    versionedVector(size_t n) : rawVector(n) {
        if (n > 0) version.bumpVersion();
    }

    versionedVector(size_t n, const T& val) : rawVector(n, val) {
        version.bumpVersion();
    }

    versionedVector(std::initializer_list<T> init) : rawVector(init) {
        version.bumpVersion();
    }

    template<typename Container>
    versionedVector(Container&& container) : rawVector(std::forward<Container>(container)) {
        version.bumpVersion();
    }

    template<typename InputIt>
    versionedVector(InputIt first, InputIt last) : rawVector(first, last) {
        version.bumpVersion();
    }

    versionedVector(std::vector<T>&& vec) noexcept : rawVector(std::move(vec)) {
        version.bumpVersion();
    }

    // Copy/move
    versionedVector(const versionedVector&) = default;
    versionedVector(versionedVector&&) noexcept = default;
    versionedVector& operator=(const versionedVector&) = default;
    versionedVector& operator=(versionedVector&&) noexcept = default;

    // Assignment operators
    versionedVector& operator=(std::initializer_list<T> init) {
        rawVector = init;
        mark_all_dirty();
        return *this;
    }

    versionedVector& operator=(const std::vector<T>& vec) {
        rawVector = vec;
        mark_all_dirty();
        return *this;
    }

    versionedVector& operator=(std::vector<T>&& vec) noexcept {
        rawVector = std::move(vec);
        mark_all_dirty();
        return *this;
    }

    // Iterators
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    //iterator begin() { return rawVector.begin(); }
    //iterator end() { return rawVector.end(); }
    const_iterator begin() const { return rawVector.begin(); }
    const_iterator end() const { return rawVector.end(); }
    const_iterator cbegin() const { return rawVector.cbegin(); }
    const_iterator cend() const { return rawVector.cend(); }

    // Element access
    T& front() {
        mark_dirty_index(0);
        return rawVector.front();
    }
    const T& front() const { return rawVector.front(); }

    T& back() {
        mark_dirty_index(size() - 1);
        return rawVector.back();
    }
    const T& back() const { return rawVector.back(); }

    // Raw data access
    //T* data() { return rawVector.data(); } // Нельзя, ткк данные при изменении версию оставят
    const T* data() const { return rawVector.data(); }

    // Capacity
    size_t size() const { return rawVector.size(); }
    size_t capacity() const { return rawVector.capacity(); }
    bool empty() const { return rawVector.empty(); }
    void reserve(size_t newCapacity) {
        rawVector.reserve(newCapacity);
    }

    // Resize operations
    void resize(size_t newSize) {
        size_t oldSize = size();
        rawVector.resize(newSize);
        if (newSize > oldSize) {
            mark_dirty_range(oldSize, newSize);  // new elements
        }
        else if (newSize < oldSize) {
            mark_dirty_range(0, newSize);  // existing elements potentially shifted? mark all
        }
    }

    void resize(size_t newSize, const T& val) {
        size_t oldSize = size();
        rawVector.resize(newSize, val);
        if (newSize > oldSize) {
            mark_dirty_range(oldSize, newSize);
        }
        else if (newSize < oldSize) {
            mark_dirty_range(0, newSize);
        }
    }

    // Erase operations
    void erase(const_iterator where) {
        size_t pos = where - rawVector.begin();
        rawVector.erase(where);
        mark_dirty_range(pos, size());
    }

    void erase(const_iterator first, const_iterator last) {
        size_t pos = first - rawVector.begin();
        rawVector.erase(first, last);
        mark_dirty_range(pos, size());
    }

    // Insert operations
    void insert(const_iterator where, const T& val) {
        size_t pos = where - rawVector.begin();
        rawVector.insert(where, val);
        mark_dirty_range(pos, size());
    }

    void insert(const_iterator where, size_t count, const T& val) {
        size_t pos = where - rawVector.begin();
        rawVector.insert(where, count, val);
        mark_dirty_range(pos, size());
    }

    void insert(const_iterator where, std::initializer_list<T> list) {
        size_t pos = where - rawVector.begin();
        rawVector.insert(where, list);
        mark_dirty_range(pos, size());
    }

    template<typename Iter>
    void insert(const_iterator where, Iter first, Iter last) {
        size_t pos = where - rawVector.begin();
        rawVector.insert(where, first, last);
        mark_dirty_range(pos, size());
    }

    void push_back(const T& val) {
        rawVector.push_back(val);
        mark_dirty_index(size() - 1);
    }

    void push_back(T&& val) {
        rawVector.push_back(std::move(val));
        mark_dirty_index(size() - 1);
    }

    void pop_back() {
        rawVector.pop_back();
        mark_dirty_range(0, size());
    }

    void clear() {
        rawVector.clear();
        mark_all_dirty();
    }

    size_t getVersion() const { return version.getVersion(); }

    auto& vec() const { return rawVector; }

    friend std::ostream& operator<<(std::ostream& os, const versionedVector& vector) {
        os << "--- versioned_vector (size=" << vector.size()
            << ", dirty=[" << vector.dirty_begin << "," << vector.dirty_end << "]) ---\n";
        for (const auto& val : vector.rawVector) {
            os << val << '\n';
        }
        return os;
    }
};


template<template<typename...> typename Vec, typename... Tags>
class attribute_vector;

template<typename... Tags>
using version_vector = attribute_vector<versionedVector, Tags...>;