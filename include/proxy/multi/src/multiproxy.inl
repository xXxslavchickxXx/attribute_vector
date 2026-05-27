#pragma once

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
template<typename AnotherProxy>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	upload(size_t where, const AnotherProxy& proxy)
	requires (!Is_const && has_tags_v<AnotherProxy>)
{
	using InnerTags = typename AnotherProxy::tags;

	auto upload_from_proxy = [&]<typename Tag>() {
		if constexpr (tuple_contains_v<Tag, InnerTags>) {
			this->upload_one<Tag>(where, proxy.vector<Tag>());
		}
	};

	(this->template call<SelectedTags>(upload_from_proxy), ...);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
template<typename AnotherProxy>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	insert(size_t where, const AnotherProxy& proxy)
	requires (!Is_const && has_tags_v<AnotherProxy>)
{
	using InnerTags = AnotherProxy::tags;
	using CurrentTags = tags;

	static_assert(tuple_is_similar<InnerTags, CurrentTags>,
		"tags of the copied proxy do not match the current proxy");

	auto insert_default = [&]<typename Tag>() {
		std::vector<typename Tag::type> default_vec(proxy.size(), Tag::defaultValue());
		this->insert_container<Tag>(where, default_vec);
	};

	auto insert_from_proxy = [&]<typename Tag>() {
		if constexpr (tuple_contains_v<Tag, InnerTags>) {
			this->insert_container<Tag>(where, proxy.vector<Tag>());
		}
		else {
			call<Tag>(insert_default);
		}
	};

	((call<SelectedTags>(insert_from_proxy)), ...);

	execute_for_other(insert_default);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::multi_proxy(TupleType& data)
	: _data(&data)
{}
template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::multi_proxy(const TupleType& data)
	: _data(&data)
{}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	push_back(const SelectedTags::type&... value) requires (!Is_const)
{
	insert(size(), value...);
}
template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	pop_back() requires (!Is_const)
{
	erase(size() - 1);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
template<typename... Containers>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	insert_containers(size_t where, const Containers&... containers) requires (!Is_const)
{
	static_assert(sizeof...(Containers) == sizeof...(SelectedTags),
		"Number of containers must match number of selected tags");

	if (where > size()) {
		throw std::out_of_range("Insert position out of range");
	}

	size_t first_size = std::get<0>(std::tie(containers...)).size();
	bool all_same = ((containers.size() == first_size) && ...);
	if (!all_same) {
		throw std::runtime_error("All containers must have the same size");
	}

	size_t max_size = 0;
	((max_size = std::max(max_size, containers.size())), ...);

	(insert_container<SelectedTags>(where, containers), ...);

	auto inserter = [&]<typename Tag>() {
		std::vector<typename Tag::type> default_vec(max_size, Tag::defaultValue());
		insert_container<Tag>(where, default_vec);
	};

	execute_for_other(inserter);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	insert_list(size_t where, const std::initializer_list<typename SelectedTags::type>&... lists) requires (!Is_const)
{
	insert_containers(where, lists...);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
template<typename... Containers>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	upload_containers(size_t where, const Containers&... containers) requires (!Is_const)
{
	static_assert(sizeof...(Containers) == sizeof...(SelectedTags),
		"Number of containers must match number of selected tags");

	size_t first_size = std::get<0>(std::tie(containers...)).size();
	bool all_same = ((containers.size() == first_size) && ...);
	if (!all_same) {
		throw std::runtime_error("These lists are not of the same size");
	}

	size_t max_size = 0;
	((max_size = std::max(max_size, containers.size())), ...);

	size_t attribute_vector_size = std::get<0>(*_data).size();

	if (max_size + where > attribute_vector_size) {
		resize(max_size + where);
	}

	((upload_one<SelectedTags>(where, containers)), ...);
}
	
template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>	
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	upload_list(size_t where, const std::initializer_list<typename SelectedTags::type>&... lists) requires (!Is_const)
{
	upload_containers(where, lists...);
}
	
template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	upload(size_t where, const SelectedTags::type&... values) requires (!Is_const)
{
	upload_list(where, { values }...);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	erase(size_t where) requires (!Is_const)
{
	erase(where, 1);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	erase(size_t where, size_t n) requires (!Is_const)
{
	size_t cur_size = size();
	if (where >= cur_size) {
		throw std::out_of_range("Erase position out of range");
	}
	if (where + n > cur_size) {
		n = cur_size - where; // можно обрезать до конца
	}
	if (n == 0) return;

	auto erase_one = [&](auto& vector) {
		auto start = vector.begin() + where;
		vector.erase(start, start + n);
		};
	execute_for_all(erase_one);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
constexpr size_t attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	size() const {
	if constexpr (sizeof...(SelectedTags) > 0) {
		return std::get<0>(*_data).size();
	}
	return 0;
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
size_t attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	capacity() const {
	if constexpr (sizeof...(SelectedTags) > 0) {
		return std::get<0>(*_data).capacity();
	}
	return 0;
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	reserve(size_t new_capacity) requires (!Is_const)
{
	auto reserve_one = [&](auto& vector) {
		vector.reserve(new_capacity);
		};

	execute_for_all(reserve_one);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	resize(size_t new_size) requires (!Is_const)
{
	if (new_size == size()) return;

	auto resize_one = [&](auto& vector) {
		vector.resize(new_size);
		};
	execute_for_all(resize_one);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	resize(size_t new_size, const SelectedTags::type&... values) requires (!Is_const)
{
	((mutable_vector<SelectedTags>().resize(new_size, values)), ...);

	auto reserve_one = [&]<typename Tag>() {
		mutable_vector<Tag>().resize(new_size, Tag::defaultValue());
	};

	execute_for_other(reserve_one);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	insert(size_t where, size_t n, const SelectedTags::type&... values) requires (!Is_const)
{
	if (where > size()) {
		throw std::out_of_range("Insert position out of range");
	}

	((insert_one<SelectedTags>(where, n, values)), ...);

	auto inserter = [&]<typename Tag>() {
		insert_one<Tag>(where, n, Tag::defaultValue());
	};

	execute_for_other(inserter);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	insert(size_t where, const SelectedTags::type&... values) requires (!Is_const)
{
	insert(where, 1, values...);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
template<typename F>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	execute_for_other(F&& func) 
{
	((!(hasTag<Tags, SelectedTags...>()) ? (call<Tags>(func), 0) : 0), ...);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
template<typename F>
void attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	execute_for_all(F&& func)
{
	std::apply([&](auto&... vectors) {
		((func(vectors)), ...);
		}, *_data);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
template<typename Tag>
const Vec<typename Tag::type>& attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	vector() const
{
	static_assert(hasTag<Tag, SelectedTags...>(),
		"This tag doesn't belong to this collection");
	return std::get<getTagIndex<Tag>()>(*_data);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
template<typename Tag>
auto& attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::mutable_vector() {
	return std::get<getTagIndex<Tag>()>(*_data);
}

template<template<typename...> typename Vec, typename... Tags>
template<bool Is_const, typename... SelectedTags>
template<typename Tag>
constexpr size_t attribute_vector<Vec, Tags...>::multi_proxy<Is_const, SelectedTags...>::
	getTagIndex() const
{
	static_assert(hasTag<Tag, Tags...>(), "tag doesn't excist in this attrib_vector");

	using Tuple = std::tuple<Tags...>;

	constexpr size_t idx = []<size_t... Is>(std::index_sequence<Is...>) {
		size_t result = 0;
		((
			std::is_same_v<Tag, std::tuple_element_t<Is, Tuple>> ? result = Is : false
			), ...);
		return result;
	}(std::index_sequence_for<Tags...>{});

	return idx;
}