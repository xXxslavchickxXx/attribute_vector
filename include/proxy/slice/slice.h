#pragma once

#include <proxy/multi/multi_proxy.h>
#include <iostream>

template<IsAttributeVector AttributeVectorT, bool IsConst, typename... SelectedTags>
class slice_proxy : public base_proxy<AttributeVectorT, IsConst, SelectedTags...> {
    using Base = base_proxy<AttributeVectorT, IsConst, SelectedTags...>;
    using Base::data_;
    using Base::PointerType;

    size_t _begin;
    size_t _end;

public:
    // Конструктор
    slice_proxy(PointerType data, size_t begin, size_t end)
        : Base(data), _begin(begin), _end(end)
    {
        if (begin > end) throw std::out_of_range("slice: begin > end");
        if (end > this->size()) throw std::out_of_range("slice: end out of range");
    }

    // Размер
    size_t size() const { return _end - _begin; }

    template<typename Tag>
    const typename Tag::type* data() const {
        return this->template vector<Tag>().data() + _begin;
    }

    template<typename Tag>
    typename Tag::type* data() requires (!IsConst) {
        return this->template mutable_vector<Tag>().data() + _begin;
    }

    size_t offset() const {
        return _begin;
    }

    auto slice(size_t sub_begin, size_t sub_end) {
        return slice_proxy<AttributeVectorT, IsConst, SelectedTags...>(
            data_, _begin + sub_begin, _begin + sub_end
        );
    }

    auto as_proxy() {
        return multi_proxy<AttributeVectorT, IsConst, SelectedTags...>(data_);
    }
};