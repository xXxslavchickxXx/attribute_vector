#pragma once

#include <tuple>

/// FWD
namespace engine::data {
	template<template<typename...> typename Vec, typename... Tags>
	class attribute_vector;
}

/// TT
template<typename T, typename = void>
struct TypeHasTags : std::false_type {};

template<typename T>
struct TypeHasTags<T, std::void_t<typename T::tags>> : std::true_type {};

template<typename T>
inline constexpr bool has_tags_v = TypeHasTags<T>::value;

template<typename T>
struct is_tuple : std::false_type {};

template<typename... Args>
struct is_tuple<std::tuple<Args...>> : std::true_type {};

template<typename T>
inline constexpr bool is_tuple_v = is_tuple<T>::value;

template<typename T, typename Tuple>
constexpr bool tuple_contains_v = []<size_t... Is>(std::index_sequence<Is...>) {
	static_assert(is_tuple_v<Tuple>, "This type isn't tuple!");
	return (std::is_same_v<T, std::tuple_element_t<Is, Tuple>> || ...);
}(std::make_index_sequence<std::tuple_size_v<Tuple>>{});

template<typename tuple1, typename tuple2>
constexpr bool tuple_is_similar = []<size_t... Is>(std::index_sequence<Is...>) {
	static_assert(is_tuple_v<tuple1> && is_tuple_v<tuple2>, "It isn't tuples");
	return (tuple_contains_v<
		std::tuple_element_t<Is, tuple1>,
		tuple2
	> || ...);
}(std::make_index_sequence<std::tuple_size_v<tuple1>>{});

template<typename tuple1, typename tuple2>
constexpr bool tags_are_same = []<size_t... Is>(std::index_sequence<Is...>) {
		static_assert(is_tuple_v<tuple1> && is_tuple_v<tuple2>, "It isn't tuples");
		static_assert(std::tuple_size_v<tuple1> == std::tuple_size_v<tuple2>, "Tuples have different sizes");
		return (tuple_contains_v<
			std::tuple_element_t<Is, tuple1>,
			tuple2
		> && ...);
	}(std::make_index_sequence<std::tuple_size_v<tuple1>>{});

template<typename Tag, typename... Tags>
constexpr bool hasTag() {
	return (std::is_same_v<Tag, Tags> || ...);
}

template<typename T>
struct is_attribute_vector : std::false_type {};

template<template<typename...> typename Vec, typename... Args>
struct is_attribute_vector<engine::data::attribute_vector<Vec, Args...>> : std::true_type {};

template<typename Tuple1, typename Tuple2>
concept SameTags = (std::tuple_size_v<Tuple1> == std::tuple_size_v<Tuple2>) &&
[]<size_t... Is>(std::index_sequence<Is...>) {
	return (tuple_contains_v<std::tuple_element_t<Is, Tuple1>, Tuple2> && ...);
}(std::make_index_sequence<std::tuple_size_v<Tuple1>>{});

template<typename T, typename = void>
struct is_single_proxy : std::false_type {};

template<typename T>
struct is_single_proxy<T, std::void_t<typename T::is_single_proxy_tag>> : std::true_type {};

template<typename T>
constexpr bool is_single_proxy_v = is_single_proxy<T>::value;