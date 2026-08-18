#pragma once

#include <proxy/base/base.h>

template<IsAttributeVector AttributeVectorT, bool IsConst, typename... SelectedTags>
class multi_proxy : public base_proxy<AttributeVectorT, IsConst, SelectedTags...> {
protected:
	using Base = base_proxy<AttributeVectorT, IsConst, SelectedTags...>;

	// Открываем допступ к родительским методам и полям
	template<typename Tag>
	using vec_type = typename Base::template vec_type<Tag>;

public:
	using typename Base::owner_tags;
	using typename Base::tags;
	using Base::Base;
	using Base::vector;
	using Base::empty;

	/// Методы вставки
	template<typename AnotherProxy>
	void insert(size_t where, const AnotherProxy& proxy)
		requires (!IsConst && has_tags_v<AnotherProxy>);

	template<typename AnotherProxy>
	void insert(size_t where, AnotherProxy&& proxy)
		requires (!IsConst && has_tags_v<AnotherProxy>);

	template<typename... Containers>
	void insert_containers(size_t where, Containers&&... containers)
		requires (!IsConst);
	void insert_list(size_t where,
		const std::initializer_list<typename SelectedTags::type>&... lists)
		requires (!IsConst);

	template<typename... T>
	void insert(size_t where, size_t n,
				T&&... values) requires (!IsConst &&
		(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...));

	template<typename... T>
	void insert(size_t where,
				T&&... values) requires (!IsConst &&
		(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...));

	/// Методы удаления
	void erase(size_t where) requires (!IsConst);
	void erase(size_t where, size_t n) requires (!IsConst);
	void clear() requires (!IsConst);

	/// Методы резервирования и ресайзов
	void reserve(size_t new_capacity) requires (!IsConst);
	void resize(size_t new_size) requires (!IsConst);

	template<typename... T>
	void resize(size_t new_size, T&&... values)
		requires (!IsConst &&
		(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...));

	/// Методы аплоада даты в текущие данные
	template<typename AnotherProxy>
	void upload(size_t where, AnotherProxy&& proxy)
		requires (!IsConst && has_tags_v<AnotherProxy>);
	template<typename AnotherProxy>
	void upload(size_t where, const AnotherProxy& proxy)
		requires (!IsConst && has_tags_v<AnotherProxy>);

	template<typename... Containers>
	void upload_containers(size_t where, Containers&&... containers)
		requires (!IsConst);
	
	template<typename... T>
	void upload_list(size_t where,
	const std::initializer_list<T>&... lists)
		requires (!IsConst &&
	(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...));

	template<typename... T>
	void upload(size_t where, T&&... values)
		requires (!IsConst &&
	(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...));

	/// Классические методы для работы с концом векторов
	template<typename... T>
	void push_back(T&&... value) requires (!IsConst &&
	(std::is_same_v<std::decay_t<T>, typename SelectedTags::type> && ...));
	void pop_back() requires (!IsConst);
	
	template<typename Tag>
	auto attribute();
	template<typename Tag>
	auto attribute() const;

	template<typename... Tags>
	auto slice(size_t begin, size_t end);
	template<typename... Tags>
	auto slice(size_t begin, size_t end) const;

	auto slice(size_t begin, size_t end);
	auto slice(size_t begin, size_t end) const;

private:
	template<typename F>
	void execute_for_other(F&& func);
	template<typename F>
	void execute_for_all(F&& func);

	template<typename Tag, typename T>
	void insert_one(size_t where, size_t n, T&& value);
	template<typename Tag, typename Container>
	void insert_container(size_t where, Container&& container);
	template<typename Tag, typename Container>
	void upload_one(size_t where, Container&& container);
};


#include "multiproxy.inl"