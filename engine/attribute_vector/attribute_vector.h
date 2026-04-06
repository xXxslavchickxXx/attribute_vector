#pragma once

#include "../versionedVector/versioned_vector.h"
#include <tuple>

namespace engine::data {
	template<typename Tag, typename... Tags>
	constexpr bool hasTag();
	
	template<template<typename...> typename Vec, typename... Tags>
	class attribute_vector {
		std::tuple<Vec<typename Tags::type>...> _data;
		
	public:
		template<typename... SelectedTags>
		auto with();
		template<typename... SelectedTags>
		auto with() const;

		template<typename Tag>
		auto attribute();

		template<typename Tag>
		auto attribute() const;

		size_t size() const;
		void show_vector() const;

	public:
		template<bool Is_const, typename... SelectedTags>
		class multi_proxy;
		template<bool IsConst, typename Tag>
		class single_proxy;

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

	/// <summary>
	/// Поленые методы
	/// </summary>
	private:
		template<size_t... Is>
		void show_vector_impl(std::index_sequence<Is...>) const;

		void validate_sizes() const;
	};
}

#include "src/attribute_vector.inl"