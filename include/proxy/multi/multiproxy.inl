#pragma once

#include <proxy/single/singleproxy.h>
#include <proxy/slice/slice.h>

#define multi_proxy_template template<IsAttributeVector AttributeVectorT, bool IsConst, typename... SelectedTags>
#define multi_proxy_type multi_proxy<AttributeVectorT, IsConst, SelectedTags...>

multi_proxy_template
void multi_proxy_type::clear() requires (!IsConst) {
	execute_for_all([&](auto& vector) {
		vector.clear();
	});
}

multi_proxy_template
auto multi_proxy_type::slice(size_t begin, size_t end) {
	return slice<SelectedTags...>(begin, end);
}
multi_proxy_template
auto multi_proxy_type::slice(size_t begin, size_t end) const {
	return slice<SelectedTags...>(begin, end);
}

multi_proxy_template
template<typename... Tags>
auto multi_proxy_type::slice(size_t begin, size_t end) {
	static_assert((hasTag<Tags, SelectedTags...>() && ...),
	"this slice requests tags that do not exist in this proxy.");
	return slice_proxy<AttributeVectorT, IsConst, Tags...>(this->data_, begin, end);
}
multi_proxy_template
template<typename... Tags>
auto multi_proxy_type::slice(size_t begin, size_t end) const {
	static_assert((hasTag<Tags, SelectedTags...>() && ...),
		"this slice requests tags that do not exist in this proxy.");
	return slice_proxy<AttributeVectorT, IsConst, Tags...>(this->data_, begin, end);
}

multi_proxy_template
template<typename Tag>
auto multi_proxy_type::attribute() {
	return single_proxy<AttributeVectorT, IsConst, Tag>(this->data_);
}
multi_proxy_template
template<typename Tag>
auto multi_proxy_type::attribute() const {
	return single_proxy<AttributeVectorT, IsConst, Tag>(this->data_);
}

multi_proxy_template
template<typename AnotherProxy>
void multi_proxy_type::upload(size_t where, const AnotherProxy& proxy)
	requires (!IsConst && has_tags_v<AnotherProxy>)
{
	using InnerTags = typename AnotherProxy::tags;

	auto upload_from_proxy = [&]<typename Tag>() {
		if constexpr (tuple_contains_v<Tag, InnerTags>) {
			this->upload_one<Tag>(where,
				proxy.template vector<Tag>());
		}
	};

	(this->template call<SelectedTags>(upload_from_proxy), ...);
}

multi_proxy_template
template<typename AnotherProxy>
void multi_proxy_type::upload(size_t where, AnotherProxy&& proxy)
	requires (!IsConst && has_tags_v<AnotherProxy>)
{
	using InnerTags = typename AnotherProxy::tags;

	// Перемещаем прокси в локальную переменную
	auto&& local_proxy = std::forward<AnotherProxy>(proxy);

	auto upload_from_proxy = [&]<typename Tag>() {
		if constexpr (tuple_contains_v<Tag, InnerTags>) {
			// Перемещаем вектор из локальной копии
			this->upload_one<Tag>(where,
				std::move(local_proxy.template mutable_vector<Tag>()));
		}
	};

	(this->template call<SelectedTags>(upload_from_proxy), ...);
}


multi_proxy_template
template<typename AnotherProxy>
void multi_proxy_type::insert(size_t where, AnotherProxy&& proxy)
	requires (!IsConst && has_tags_v<AnotherProxy>)
{
	using InnerTags = AnotherProxy::tags;
	using CurrentTags = Base::tags;

	static_assert(tuple_is_similar<InnerTags, CurrentTags>,
		"tags of the copied proxy do not match the current proxy");

	auto insert_default = [&]<typename Tag>() {
		std::vector<typename Tag::type> default_vec(proxy.size(), typename Tag::type());
		this->insert_container<Tag>(where, std::move(default_vec));
	};

	auto insert_from_proxy = [&]<typename Tag>() {
		if constexpr (tuple_contains_v<Tag, InnerTags>) {
			this->insert_container<Tag>(where,
				std::move(proxy.template mutable_vector<Tag>()));
		}
		else {
			this->template call<Tag>(insert_default);
		}
	};

	((this->template call<SelectedTags>(insert_from_proxy)), ...);

	execute_for_other(insert_default);
}

multi_proxy_template
template<typename AnotherProxy>
void multi_proxy_type::insert(size_t where, const AnotherProxy& proxy)
	requires (!IsConst && has_tags_v<AnotherProxy>)
{
	using InnerTags = AnotherProxy::tags;
	using CurrentTags = Base::tags;

	static_assert(tuple_is_similar<InnerTags, CurrentTags>,
		"tags of the copied proxy do not match the current proxy");

	auto insert_default = [&]<typename Tag>() {
		std::vector<typename Tag::type> default_vec(proxy.size(), typename Tag::type());
		this->insert_container<Tag>(where, std::move(default_vec));
	};

	auto insert_from_proxy = [&]<typename Tag>() {
		if constexpr (tuple_contains_v<Tag, InnerTags>) {
			this->insert_container<Tag>(where, proxy.template vector<Tag>());
		}
		else {
			this->template call<Tag>(insert_default);
		}
	};

	((this->template call<SelectedTags>(insert_from_proxy)), ...);

	execute_for_other(insert_default);
}

multi_proxy_template
template<typename... T>
void multi_proxy_type::push_back(T&&... value) requires (!IsConst &&
	(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...))
{

	insert(this->size(), std::forward<T>(value)...);
}
multi_proxy_template
void multi_proxy_type::pop_back() requires (!IsConst)
{
	erase(this->size() - 1);
}

multi_proxy_template
template<typename... Containers>
void multi_proxy_type::insert_containers(size_t where, Containers&&... containers) requires (!IsConst)
{
	static_assert(sizeof...(Containers) == sizeof...(SelectedTags),
		"Number of containers must match number of selected tags");

	if (where > this->size()) {
		throw std::out_of_range("Insert position out of range");
	}

	size_t first_size = std::get<0>(std::tie(containers...)).size();
	bool all_same = ((containers.size() == first_size) && ...);
	if (!all_same) {
		throw std::runtime_error("All containers must have the same size");
	}

	size_t max_size = 0;
	((max_size = std::max(max_size, containers.size())), ...);

	(insert_container<SelectedTags>(where,
		std::forward<Containers>(containers)), ...);

	auto inserter = [&]<typename Tag>() {
		vec_type<Tag> default_vec(max_size, typename Tag::type());
		insert_container<Tag>(where, default_vec);
	};

	execute_for_other(inserter);
}

multi_proxy_template
void multi_proxy_type::insert_list(size_t where, const std::initializer_list<typename SelectedTags::type>&... lists) requires (!IsConst)
{
	insert_containers(where, lists...);
}

multi_proxy_template
template<typename... Containers>
void multi_proxy_type::upload_containers(size_t where, Containers&&... containers) requires (!IsConst)
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

	size_t attribute_vector_size = std::get<0>(*this->data_).size();

	if (max_size + where > attribute_vector_size) {
		resize(max_size + where);
	}

	((upload_one<SelectedTags>(where, std::forward<Containers>(containers))), ...);
}
	
multi_proxy_template
template<typename... T>
void multi_proxy_type::upload_list(size_t where,
	const std::initializer_list<T>&... lists)
	requires (!IsConst &&
(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...))
{
	upload_containers(where, lists...);
}
	
multi_proxy_template
template<typename... T>
void multi_proxy_type::upload(size_t where,
	T&&... values) requires (!IsConst &&
	(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...))
{
	if (where >= this->size()) {
		this->resize(where + 1);
	}

	((this->template mutable_vector<SelectedTags>()[where] = std::forward<T>(values)),
	...);
}

multi_proxy_template
void multi_proxy_type::erase(size_t where) requires (!IsConst)
{
	erase(where, 1);
}

multi_proxy_template
void multi_proxy_type::erase(size_t where, size_t n) requires (!IsConst)
{
	size_t cur_size = this->size();
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

multi_proxy_template
void multi_proxy_type::reserve(size_t new_capacity) requires (!IsConst)
{
	auto reserve_one = [&](auto& vector) {
		vector.reserve(new_capacity);
		};

	execute_for_all(reserve_one);
}

multi_proxy_template
void multi_proxy_type::resize(size_t new_size) requires (!IsConst)
{
	if (new_size == this->size()) return;

	auto resize_one = [&](auto& vector) {
		vector.resize(new_size);
		};
	execute_for_all(resize_one);
}

multi_proxy_template
template<typename... T>
void multi_proxy_type::resize(size_t new_size, T&&... values)
	requires (!IsConst &&
	(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...))
{
	auto values_tuple = std::forward_as_tuple(std::forward<T>(values)...);

	[&] <size_t... Is>(std::index_sequence<Is...>) {
		((this->template mutable_vector<SelectedTags>().resize(new_size,
		 std::get<Is>(values_tuple))), ...);
	}(std::index_sequence_for<T...>{});
}

multi_proxy_template
template<typename... T>
void multi_proxy_type::insert(size_t where, size_t n, T&&... values)
	requires (!IsConst &&
	(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...))
{
	if (where > this->size()) {
		throw std::out_of_range("Insert position out of range");
	}

	((insert_one<SelectedTags>(where, n, std::forward<T>(values))), ...);

	auto inserter = [&]<typename Tag>() {
		insert_one<Tag>(where, n, typename Tag::type());
	};

	execute_for_other(inserter);
}

multi_proxy_template
template<typename... T>
void multi_proxy_type::insert(size_t where, T&&... values)
	requires (!IsConst &&
	(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...))
{
	insert(where, 1, std::forward<T>(values)...);
}

multi_proxy_template
template<typename F>
void multi_proxy_type::execute_for_other(F&& func) 
{
	auto do_if_tag_not_in_my_set = [&]<size_t Index>() {
		using Tag = std::tuple_element_t<Index, owner_tags>;
		if constexpr (!tuple_contains_v<Tag, tags>) {
			this->template call<Tag>(func);
		}
	};

	[&]<size_t... Is>(std::index_sequence<Is...>) {
		(do_if_tag_not_in_my_set.template operator()<Is> (), ...);
	} (std::make_index_sequence<std::tuple_size_v<owner_tags>>{});
}

multi_proxy_template
template<typename F>
void multi_proxy_type::execute_for_all(F&& func)
{
	std::apply([&](auto&... vectors) {
		((func(vectors)), ...);
		}, *this->data_);
}

multi_proxy_template
template<typename Tag, typename T>
void multi_proxy_type::insert_one(size_t where, size_t n, T&& value) {
	static_assert(std::is_same_v<std::decay_t<T>, typename Tag::type>,
					  "Type mismatch!");

	auto& vec = this->template mutable_vector<Tag>();

	vec.insert(vec.begin() + where, n, std::forward<T>(value));
}
multi_proxy_template
template<typename Tag, typename Container>
void multi_proxy_type::insert_container(size_t where, Container&& container) {
	auto& vec = this->template mutable_vector<Tag>();

	auto&& c = std::forward<Container>(container);
	vec.insert(vec.begin() + where,
		std::make_move_iterator(c.begin()),
		std::make_move_iterator(c.end()));
}
multi_proxy_template
template<typename Tag, typename Container>
void multi_proxy_type::upload_one(size_t where, Container&& container) {
	auto& vec = this->template mutable_vector<Tag>();
	size_t needed_size = where + container.size();

	if (needed_size > vec.size()) {
		this->resize(needed_size);
	}

	auto&& c = std::forward<Container>(container);
	std::copy(
		std::make_move_iterator(c.begin()),
		std::make_move_iterator(c.end()),
		vec.begin() + where);
}