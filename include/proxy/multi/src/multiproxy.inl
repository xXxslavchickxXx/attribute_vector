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
template<typename... Tags>
auto multi_proxy_type::slice(size_t begin, size_t end) {
	static_assert((hasTag<Tags, SelectedTags...>() && ...),
	"this slice requests tags that do not exist in this proxy.");
	return slice_proxy<AttributeVectorT, false, Tags...>(&data_, begin, end);
}
multi_proxy_template
template<typename... Tags>
auto multi_proxy_type::slice(size_t begin, size_t end) const {
	static_assert((hasTag<Tags, SelectedTags...>() && ...),
		"this slice requests tags that do not exist in this proxy.");
	return slice_proxy<AttributeVectorT, true, Tags...>(&data_, begin, end);
}

multi_proxy_template
template<typename Tag>
auto multi_proxy_type::attribute() {
	return single_proxy<AttributeVectorT, false, Tag>(data_);
}
multi_proxy_template
template<typename Tag>
auto multi_proxy_type::attribute() const {
	return single_proxy<AttributeVectorT, true, Tag>(data_);
}

multi_proxy_template
template<typename AnotherProxy>
void multi_proxy_type::upload(size_t where, const AnotherProxy& proxy)
	requires (!IsConst && has_tags_v<AnotherProxy>)
{
	using InnerTags = typename AnotherProxy::tags;

	auto upload_from_proxy = [&]<typename Tag>() {
		if constexpr (tuple_contains_v<Tag, InnerTags>) {
			this->upload_one<Tag>(where, proxy.vector<Tag>());
		}
	};

	(this->template call<SelectedTags>(upload_from_proxy), ...);
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

multi_proxy_template
void multi_proxy_type::push_back(const SelectedTags::type&... value) requires (!IsConst)
{
	insert(this->size(), value...);
}
multi_proxy_template
void multi_proxy_type::pop_back() requires (!IsConst)
{
	erase(this->size() - 1);
}

multi_proxy_template
template<typename... Containers>
void multi_proxy_type::insert_containers(size_t where, const Containers&... containers) requires (!IsConst)
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

	(insert_container<SelectedTags>(where, containers), ...);

	auto inserter = [&]<typename Tag>() {
		vec_type<Tag> default_vec(max_size, Tag::defaultValue());
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
void multi_proxy_type::upload_containers(size_t where, const Containers&... containers) requires (!IsConst)
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

	size_t attribute_vector_size = std::get<0>(*data_).size();

	if (max_size + where > attribute_vector_size) {
		resize(max_size + where);
	}

	((upload_one<SelectedTags>(where, containers)), ...);
}
	
multi_proxy_template
void multi_proxy_type::upload_list(size_t where, const std::initializer_list<typename SelectedTags::type>&... lists) requires (!IsConst)
{
	upload_containers(where, lists...);
}
	
multi_proxy_template
void multi_proxy_type::upload(size_t where, const SelectedTags::type&... values) requires (!IsConst)
{
	upload_list(where, { values }...);
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
void multi_proxy_type::resize(size_t new_size, const SelectedTags::type&... values) requires (!IsConst)
{
	((mutable_vector<SelectedTags>().resize(new_size, values)), ...);

	auto reserve_one = [&]<typename Tag>() {
		mutable_vector<Tag>().resize(new_size, Tag::defaultValue());
	};

	execute_for_other(reserve_one);
}

multi_proxy_template
void multi_proxy_type::insert(size_t where, size_t n, const SelectedTags::type&... values) requires (!IsConst)
{
	if (where > this->size()) {
		throw std::out_of_range("Insert position out of range");
	}

	((insert_one<SelectedTags>(where, n, values)), ...);

	auto inserter = [&]<typename Tag>() {
		insert_one<Tag>(where, n, Tag::defaultValue());
	};

	execute_for_other(inserter);
}

multi_proxy_template
void multi_proxy_type::insert(size_t where, const SelectedTags::type&... values) requires (!IsConst)
{
	insert(where, 1, values...);
}

multi_proxy_template
template<typename F>
void multi_proxy_type::execute_for_other(F&& func) 
{
	auto do_if_tag_not_in_my_set = [&]<size_t Index>() {
		using Tag = std::tuple_element_t<Index, owner_tags>;
		if constexpr (!tuple_contains_v<Tag, tags>) {
			call<Tag>(func);
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
		}, *data_);
}

multi_proxy_template
template<typename Tag>
void multi_proxy_type::insert_one(size_t where, size_t n, const Tag::type& value) {
	auto& vec = mutable_vector<Tag>();
	vec.insert(vec.begin() + where, n, value);
}
multi_proxy_template
template<typename Tag, typename Container>
void multi_proxy_type::insert_container(size_t where, const Container& container) {
	auto& vec = mutable_vector<Tag>();
	vec.insert(vec.begin() + where, container.begin(), container.end());
}
multi_proxy_template
template<typename Tag, typename Container>
void multi_proxy_type::upload_one(size_t where, const Container& container) {
	auto& vec = mutable_vector<Tag>();
	// Проверка типа итератора
	static_assert(!std::is_const_v<std::remove_reference_t<decltype(*vec.begin())>>,
		"vec.begin() returns const_iterator!");

	std::copy(container.begin(), container.end(), vec.begin() + where);
}