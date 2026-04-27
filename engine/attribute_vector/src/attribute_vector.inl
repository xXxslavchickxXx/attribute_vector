#pragma once

#include <tuple>
#include "../../proxy/multi/multi_proxy.h"
#include "../../proxy/single/singleproxy.h"

namespace engine::data {
	template<template<typename...> typename Vec, typename... Tags>
	template<template<typename...> typename AnotherVec, typename... AnotherTags>
	void attribute_vector<Vec, Tags...>::
		insert(size_t where, const attribute_vector<AnotherVec, AnotherTags...>& vec)
	{
		static_assert(SameTags<std::tuple<Tags...>, std::tuple<AnotherTags...>>, 
			"The tags of the copied attribute_vector do not match the current attribute_vector.");
		this->with<Tags...>().insert(where, vec.with<AnotherTags...>());
	}

	template<template<typename...> typename Vec, typename... Tags>
	template<typename Tag>
	auto attribute_vector<Vec, Tags...>::attribute() {
		return single_proxy<false, Tag>(_data);
	}

	template<template<typename...> typename Vec, typename... Tags>
	template<typename Tag>
	auto attribute_vector<Vec, Tags...>::attribute() const {
		return single_proxy<true, Tag>(_data);
	}

	template<template<typename...> typename Vec, typename... Tags>
	attribute_vector<Vec, Tags...>::attribute_vector(size_t n)
		:
		_data(std::make_tuple(Vec<typename Tags::type>(n, Tags::defaultValue())...))
	{}

	template<template<typename...> typename Vec, typename... Tags>
	attribute_vector<Vec, Tags...>::attribute_vector(std::initializer_list<typename Tags::type>... lists)
		: _data(Vec<typename Tags::type>(lists)...)
	{
		validate_sizes();
	}

	template<template<typename...> typename Vec, typename... Tags>
	template<typename... Containers>
	attribute_vector<Vec, Tags...>::attribute_vector(Containers&&... containers) requires(sizeof...(containers) == sizeof...(Tags))
		: _data(Vec<typename Tags::type>(std::forward<Containers>(containers))...)
	{
		validate_sizes();
	}

	template<template<typename...> typename Vec, typename... Tags>
	void attribute_vector<Vec, Tags...>::show_vector() const {
		show_vector_impl(std::index_sequence_for<Tags...>{});
	}
	template<template<typename...> typename Vec, typename... Tags>
	template<size_t... Is>
	void attribute_vector<Vec, Tags...>::show_vector_impl(std::index_sequence<Is...>) const {
		((std::cout << std::get<Is>(_data)), ...);
	}

	template<template<typename...> typename Vec, typename... Tags>
	template<typename... SelectedTags>
	auto attribute_vector<Vec, Tags...>::with() {
		static_assert((hasTag<SelectedTags, Tags...>() && ...),
			"one of the tags is not in the collection");

		return multi_proxy<false, SelectedTags...>(_data);
	}

	template<template<typename...> typename Vec, typename... Tags>
	template<typename... SelectedTags>
	const auto attribute_vector<Vec, Tags...>::with() const {
		static_assert((hasTag<SelectedTags, Tags...>() && ...),
			"one of the tags is not in the collection");

		return multi_proxy<true, SelectedTags...>(_data);
	}

	template<template<typename...> typename Vec, typename... Tags>
	inline size_t attribute_vector<Vec, Tags...>::size() const {
		return std::get<0>(_data).size();
	}

	template<template<typename...> typename Vec, typename... Tags>
	void attribute_vector<Vec, Tags...>::validate_sizes() const {
		std::apply([](const auto&... vectors) {
			auto first_size = std::get<0>(std::tie(vectors...)).size();
			bool all_same = ((vectors.size() == first_size) && ...);

			if (!all_same) {
				throw std::runtime_error(
					"All containers in attribute_vector must have the same size"
				);
			}
			}, _data);
	}
}