#pragma once

#include <tuple>
#include <type_traits>

#include "../../attribute_vector/attribute_vector.h"

namespace engine::data {
	template<template<typename...> typename Vec, typename... Tags>
	template<bool Is_const, typename... SelectedTags>
	class attribute_vector<Vec, Tags...>::multi_proxy {
		// В зависимости от значение Is_const выбираем тип прокси
		using TupleType = std::tuple<Vec<typename Tags::type>...>;
		using PointerType = std::conditional_t<Is_const, const TupleType*, TupleType*>;

		PointerType _data;

	public:
		using tags = std::tuple<SelectedTags...>;

		multi_proxy(TupleType& data);
		multi_proxy(const TupleType& data);

		template<typename Tag>
		const Vec<typename Tag::type>& vector() const;

		/// Методы вставки
		template<typename AnotherProxy>
		void insert(size_t where, const AnotherProxy& proxy)
			requires (!Is_const && has_tags_v<AnotherProxy>);

		template<typename... Containers>
		void insert_containers(size_t where, const Containers&... containers) requires (!Is_const);
		void insert_list(size_t where, const std::initializer_list<typename SelectedTags::type>&... lists) requires (!Is_const);
		void insert(size_t where, size_t n, const SelectedTags::type&... values) requires (!Is_const);
		void insert(size_t where, const SelectedTags::type&... values) requires (!Is_const);

		/// Методы удаления
		void erase(size_t where) requires (!Is_const);
		void erase(size_t where, size_t n) requires (!Is_const);

		/// Методы резервирования и ресайзов
		void reserve(size_t new_capacity) requires (!Is_const);
		void resize(size_t new_size) requires (!Is_const);
		void resize(size_t new_size, const SelectedTags::type&... values) requires (!Is_const);

		/// Методы аплоада даты в текущие данные
		template<typename AnotherProxy>
		void upload(size_t where, const AnotherProxy& proxy) requires (!Is_const && has_tags_v<AnotherProxy>);

		template<typename... Containers>
		void upload_containers(size_t where, const Containers&... containers) requires (!Is_const);
		void upload_list(size_t where, const std::initializer_list<typename SelectedTags::type>&... lists) requires (!Is_const);
		void upload(size_t where, const SelectedTags::type&... values) requires (!Is_const);

		/// Классические методы для работы с концом векторов
		void push_back(const SelectedTags::type&... value) requires (!Is_const);
		void pop_back() requires (!Is_const);

	private:
		template<typename Tag>
		auto& mutable_vector();

		template<typename Tag>
		constexpr size_t getTagIndex() const;

		template<typename F>
		void execute_for_other(F&& func);

		template<typename F>
		void execute_for_all(F&& func);

	public:
		constexpr size_t size() const;
		size_t capacity() const;

	/// Реализации внутренних методов, то что пользователь не должен видеть вообще
	private:
		template<typename Tag>
		void insert_one(size_t where, size_t n, const Tag::type& value) {
			auto& vec = mutable_vector<Tag>();

			vec.insert(vec.begin() + where, n, value);
		}
		template<typename Tag, typename Container>
		void insert_container(size_t where, const Container& container) {
			auto& vec = mutable_vector<Tag>();

			vec.insert(vec.begin() + where, container.begin(), container.end());
		}
		template<typename Tag, typename Container>
		void upload_one(size_t where, const Container& container) {

			auto& vec = mutable_vector<Tag>();

			// Проверка типа итератора
			static_assert(!std::is_const_v<std::remove_reference_t<decltype(*vec.begin())>>,
				"vec.begin() returns const_iterator!");

			std::copy(container.begin(), container.end(), vec.begin() + where);
		}

		template<bool OtherIsConst, typename OtherTag>
		friend class single_proxy;

		// Метод для вызова шаблонных лямбд выражений
		// Отдельно - для того что скрыть ужасный синтаксис
		template<typename Tag, typename F>
		constexpr void call(F&& f) {
			f.template operator()<Tag>();
		}

		friend std::ostream& operator<<(std::ostream& os, const multi_proxy& proxy) {
			((os << proxy.vector<SelectedTags>() << '\n'), ...);
			return os;
		}
		friend std::ostream& operator<<(std::ostream& os, multi_proxy&& proxy) {
			((os << proxy.vector<SelectedTags>() << '\n'), ...);
			return os;
		}
	};
}

#include "src/multiproxy.inl"