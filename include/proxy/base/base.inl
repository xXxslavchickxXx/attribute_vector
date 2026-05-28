#pragma once

#define base_proxy_template template<IsAttributeVector AttributeVectorT, bool IsConst, typename... SelectedTags>
#define base_proxy_type base_proxy<AttributeVectorT, IsConst, SelectedTags...>

base_proxy_template
base_proxy_type::base_proxy(PointerType data) : data_(data) {
	// Делаем сразу все проверки, чтобы не делать их в остальных методах
	static_assert(is_attribute_vector_v<AttributeVectorT>,
	"this proxy can only be used with attribute_vector derived classes");

	static_assert((tuple_contains_v<SelectedTags, owner_tags> && ...),
	"this proxy can only process the tags that the owner has.");
}
base_proxy_template
base_proxy_type::base_proxy(DataType& data) : base_proxy(&data) { }
base_proxy_template
base_proxy_type::base_proxy(const DataType& data) : base_proxy(&data) {}


base_proxy_template
size_t base_proxy_type::size() const { return std::get<0>(*data_).size(); }
base_proxy_template
size_t base_proxy_type::capacity() const { return std::get<0>(*data_).capacity(); }
base_proxy_template
bool base_proxy_type::empty() const { return size() == 0; }

base_proxy_template
template<typename Tag>
const base_proxy_type::vec_type<Tag>& base_proxy_type::vector() const {
	return std::get<get_tag_index<Tag>()>(*data_);
}

base_proxy_template
template<typename Tag>
base_proxy_type::vec_type<Tag>& base_proxy_type::mutable_vector() {
	return std::get<get_tag_index<Tag>()>(*data_);
}

base_proxy_template
template<typename Tag>
constexpr size_t base_proxy_type::get_tag_index() const {
	static_assert(tuple_contains_v<Tag, owner_tags>,
	"this proxy doesn't contain this tag");

	constexpr size_t result = []<size_t... Is>(std::index_sequence<Is...>) {
		size_t idx = 0;
		((std::is_same_v<Tag, std::tuple_element_t<Is, owner_tags>> ? (idx = Is, true) : false), ...);
		return idx;
	}(std::make_index_sequence<std::tuple_size_v<owner_tags>>{});

	return result;
}