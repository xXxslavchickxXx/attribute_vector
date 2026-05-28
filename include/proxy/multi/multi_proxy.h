#pragma once

#include <proxy/base/base.h>

template<IsAttributeVector AttributeVectorT, bool IsConst, typename... SelectedTags>
class multi_proxy : public base_proxy<AttributeVectorT, IsConst, SelectedTags...> {
protected:
	using Base = base_proxy<AttributeVectorT, IsConst, SelectedTags...>;

	// Открываем допступ к родительским методам и полям
	template<typename Tag>
	using vec_type = typename Base::template vec_type<Tag>;
	using Base::data_;
	using Base::call;
	using Base::mutable_vector;

public:
	using Base::Base;
	using Base::tags;
	using Base::owner_tags;
	using Base::vector;

	/// Методы вставки
	template<typename AnotherProxy>
	void insert(size_t where, const AnotherProxy& proxy)
		requires (!IsConst && has_tags_v<AnotherProxy>);

	template<typename... Containers>
	void insert_containers(size_t where, const Containers&... containers) requires (!IsConst);
	void insert_list(size_t where, const std::initializer_list<typename SelectedTags::type>&... lists) requires (!IsConst);
	void insert(size_t where, size_t n, const SelectedTags::type&... values) requires (!IsConst);
	void insert(size_t where, const SelectedTags::type&... values) requires (!IsConst);

	/// Методы удаления
	void erase(size_t where) requires (!IsConst);
	void erase(size_t where, size_t n) requires (!IsConst);

	/// Методы резервирования и ресайзов
	void reserve(size_t new_capacity) requires (!IsConst);
	void resize(size_t new_size) requires (!IsConst);
	void resize(size_t new_size, const SelectedTags::type&... values) requires (!IsConst);

	/// Методы аплоада даты в текущие данные
	template<typename AnotherProxy>
	void upload(size_t where, const AnotherProxy& proxy) requires (!IsConst && has_tags_v<AnotherProxy>);

	template<typename... Containers>
	void upload_containers(size_t where, const Containers&... containers) requires (!IsConst);
	void upload_list(size_t where, const std::initializer_list<typename SelectedTags::type>&... lists) requires (!IsConst);
	void upload(size_t where, const SelectedTags::type&... values) requires (!IsConst);

	/// Классические методы для работы с концом векторов
	void push_back(const SelectedTags::type&... value) requires (!IsConst);
	void pop_back() requires (!IsConst);
	
	template<typename Tag>
	auto attribute();
	template<typename Tag>
	auto attribute() const;

	template<typename... Tags>
	auto slice(size_t begin, size_t end);
	template<typename... Tags>
	auto slice(size_t begin, size_t end) const;

private:
	template<typename F>
	void execute_for_other(F&& func);
	template<typename F>
	void execute_for_all(F&& func);

	template<typename Tag>
	void insert_one(size_t where, size_t n, const Tag::type& value);
	template<typename Tag, typename Container>
	void insert_container(size_t where, const Container& container);
	template<typename Tag, typename Container>
	void upload_one(size_t where, const Container& container);
};


#include "src/multiproxy.inl"