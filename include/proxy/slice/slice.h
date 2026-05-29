#pragma once

#include <proxy/multi/multi_proxy.h>
#include <iostream>
#include <span>

template<IsAttributeVector AttributeVectorT, bool IsConst, typename... SelectedTags>
class slice_proxy : public base_proxy<AttributeVectorT, IsConst, SelectedTags...> {
    using Base = base_proxy<AttributeVectorT, IsConst, SelectedTags...>;
    using Base::data_;
    using Base::PointerType;
    using Base::get_tag_index;

    template<typename Tag>
    using span_tag = std::conditional_t<IsConst,
        std::span<const typename Tag::type>,
        std::span<typename Tag::type>>;

    size_t _begin;
    size_t _end;

public:

    // Конструктор
    slice_proxy(PointerType data, size_t begin, size_t end)
        : Base(data), _begin(begin), _end(end)
    {
        if (begin > end) throw std::out_of_range("slice: begin > end");
        if (end > Base::size()) throw std::out_of_range("slice: end out of range");
    }

    template<typename Tag>
    span_tag<Tag> vector() {
        return span_tag<Tag>(std::get<get_tag_index<Tag>()>(*data_).data() + _begin, size());
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