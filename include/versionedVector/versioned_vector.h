#pragma once

#include <vector>
#include <iostream>

namespace versions {
	class Versioned {
		unsigned int version = 0;

	public:
		void bumpVersion() {
			version++;
		}
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
		this->_value = new_val;
		_func();
		return *this;
	}
	Field& operator=(T&& new_val) {
		this->_value = new_val;
		_func();
		return *this;
	}
	bool operator==(const T& other) const {
		return _value == other;
	}

	bool operator!=(const T& other) const {
		return _value != other;
	}

	bool operator<(const T& other) const {
		return _value < other;
	}

	bool operator>(const T& other) const {
		return _value > other;
	}

	bool operator<=(const T& other) const {
		return _value <= other;
	}

	bool operator>=(const T& other) const {
		return _value >= other;
	}

	operator const T& () const {
		return _value;
	}
};

template<typename T>
class versionedVector {
	std::vector<T> rawVector;
	versions::Versioned version;

public:
	auto operator[](size_t i) {
		auto bump = [this]() { version.bumpVersion(); };
		return Field<T, decltype(bump)>(std::move(bump), rawVector[i]);
	}
	const T& operator[](size_t i) const {
		return rawVector[i];
	}

	operator const std::vector<T>& () const { return rawVector; }

	versionedVector() = default;

	versionedVector(size_t n)
		: rawVector(n)
	{
		version.bumpVersion();
	}

	versionedVector(std::initializer_list<T> init)
		: rawVector(init)
	{
		version.bumpVersion();
	}

	template<typename Container>
	versionedVector(Container&& container)
		: rawVector(std::forward<Container>(container))
	{
		version.bumpVersion();
	}

	// Конструктор от итераторов
	template<typename InputIt>
	versionedVector(InputIt first, InputIt last)
		: rawVector(first, last)
	{
		version.bumpVersion();
	}

	// Конструктор с заполнением
	versionedVector(size_t n, const T& val)
		: rawVector(n, val)
	{
		version.bumpVersion();
	}

	// Move-конструктор от std::vector
	versionedVector(std::vector<T>&& vec) noexcept
		: rawVector(std::move(vec))
	{
		version.bumpVersion();
	}

	// Копирование и перемещение
	versionedVector(const versionedVector&) = default;
	versionedVector(versionedVector&&) noexcept = default;
	versionedVector& operator=(const versionedVector&) = default;
	versionedVector& operator=(versionedVector&&) noexcept = default;

	// Оператор присваивания от initializer_list
	versionedVector& operator=(std::initializer_list<T> init) {
		rawVector = init;
		version.bumpVersion();
		return *this;
	}

	// Оператор присваивания от std::vector
	versionedVector& operator=(const std::vector<T>& vec) {
		rawVector = vec;
		version.bumpVersion();
		return *this;
	}

	versionedVector& operator=(std::vector<T>&& vec) noexcept {
		rawVector = std::move(vec);
		version.bumpVersion();
		return *this;
	}

	using iterator = std::vector<T>::iterator;

	iterator begin() {
		return rawVector.begin();
	}

	iterator end() {
		return rawVector.end();
	}

	using const_iterator = std::vector<T>::const_iterator;

	const_iterator begin() const {
		return rawVector.begin();
	}

	const_iterator end() const {
		return rawVector.end();
	}

	const_iterator cbegin() const {
		return rawVector.cbegin();
	}

	const_iterator cend() const {
		return rawVector.cend();
	}

	T& front() {
		return rawVector.front();
	}

	const T& front() const {
		return rawVector.front();
	}

	T& back() {
		return rawVector.back();
	}

	const T& back() const {
		return rawVector.back();
	}

	auto& data() {
		return rawVector.data();
	}
	const auto& data() const {
		return rawVector.data();
	}

	// методы этого класса
	size_t getVersion() const {
		return version.getVersion();
	}

	size_t capacity() const {
		return rawVector.capacity();
	}

	bool empty() const {
		return rawVector.empty();
	}

	// ерейзы
	void erase(const_iterator where) {
		rawVector.erase(where);
	}
	void erase(const_iterator first, const_iterator last) {
		rawVector.erase(first, last);
	}

	// ресайзы
	void resize(size_t newSize) {
		rawVector.resize(newSize);
		version.bumpVersion();
	}
	void resize(size_t newSize, const T& val) {
		rawVector.resize(newSize, val);
		version.bumpVersion();
	}

	// резервы
	void reserve(size_t newCapacity) {
		rawVector.reserve(newCapacity);
		version.bumpVersion();
	}

	// Инсерты
	void insert(const_iterator where, const T& val) {
		rawVector.insert(where, val);
		version.bumpVersion();
	}
	void insert(const_iterator where, const size_t count, const T& val) {
		rawVector.insert(where, count, val);
		version.bumpVersion();
	}
	void insert(const_iterator where, std::initializer_list<T> list) {
		rawVector.insert(where, list);
		version.bumpVersion();
	}
	template<typename Iter>
	void insert(const_iterator where, Iter first, Iter last) {
		rawVector.insert(where, first, last);
		version.bumpVersion();
	}

	void push_back(const T& val) {
		rawVector.push_back(val);
		version.bumpVersion();
	}
	void pop_back() {
		rawVector.pop_back();
		version.bumpVersion();
	}

	void clear() {
		rawVector.clear();
		version.bumpVersion();
	}

	size_t size() const {
		return rawVector.size();
	}

	auto& vec() const {
		return rawVector;
	}

	friend std::ostream& operator<<(std::ostream& os, const versionedVector& vector) {
		os << "--- version_vector ---\n";
		os << vector.vec() << '\n';
		return os;
	}
};

template<template<typename...> typename Vec, typename... Tags>
class attribute_vector;

template<typename... Tags>
using version_vector = attribute_vector<versionedVector, Tags...>;