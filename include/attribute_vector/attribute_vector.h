#pragma once

#include "../versionedVector/versioned_vector.h"
#include "../type_traits/type_traits.h"
#include <proxy/base/base.h>

template<IsAttributeVector AttributeVectorT, bool IsConst, typename Tag>
class single_proxy;

template<template<typename...> typename Vec, typename... Tags>
class attribute_vector {
	std::tuple<Vec<typename Tags::type>...> _data;

	using self = attribute_vector<Vec, Tags...>;
		
public:
	template<typename... SelectedTags>
	auto get_base() {
		return base_proxy<self, false, SelectedTags...>(&_data);
	}

	template<typename... SelectedTags>
	auto with();
	template<typename... SelectedTags>
	auto with() const;

	template<typename Tag>
	auto attribute();
	template<typename Tag>
	auto attribute() const;

	template<typename... SelectedTags>
	auto slice(size_t begin, size_t end);
	template<typename... SelectedTags>
	auto slice(size_t begin, size_t end) const;

	size_t size() const;
	size_t capacity() const;

	void show_vector() const;

	template<template<typename...> typename AnotherVec, typename... AnotherTags>
	void insert(size_t where, const attribute_vector<AnotherVec, AnotherTags...>& vec);

	void erase(size_t where);
	void erase(size_t where, size_t count);

public:
	using data_type = std::tuple<Vec<typename Tags::type>...>;
	using tags = std::tuple<Tags...>;
	template<typename T>
	using vec_type = Vec<T>;

/// <summary>
/// Конструкторы, которые могут
/// принимать рвалуе листы, а также
/// любые контейнеры, от векторов
/// до эрреев
/// </summary>
public:
	attribute_vector() = default;
	attribute_vector(size_t n);
	attribute_vector(std::initializer_list<typename Tags::type>... lists);
	template<typename... Containers>
	attribute_vector(Containers&&... containers) requires(sizeof...(containers) == sizeof...(Tags));

	friend std::ostream& operator<<(std::ostream& os, const attribute_vector& vec) {
		os << vec.with<Tags...>();
		return os;
	}

/// <summary>
/// Поленые методы
/// </summary>
private:
	template<size_t... Is>
	void show_vector_impl(std::index_sequence<Is...>) const;

	void validate_sizes() const;
};

template<typename... Tags>
using default_vector = attribute_vector<std::vector, Tags...>;

#include "src/attribute_vector.inl"