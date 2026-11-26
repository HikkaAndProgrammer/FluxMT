#include "fluxmt/query/layout.hpp"

// std
#include <algorithm>

// fluxmt
#include <fluxmt/query/error.hpp>


// Implementation --------------------------------------------------------------

// Observers

flux::impl::slot_info_view flux::impl::Layout::bindings() const noexcept {
    if (const auto* fixed = std::get_if<slot_info_view>(&_repr))
        return *fixed;

    return std::get<slot_info_array>(_repr);
}

ers::maybe<size_t> flux::impl::Layout::index_of(slot_ref slot) const noexcept {
    auto view = bindings();
    auto it = std::ranges::find_if(view, [&](const slot_info_t& candidate) {
        return same_slot(candidate.slot, slot);
    });

    if (it == view.end())
        return ers::none;

    return static_cast<size_t>(it - view.begin());
}

bool flux::impl::Layout::contains(slot_ref slot) const noexcept {
    return index_of(slot).has_value();
}


// Modifiers

std::vector<flux::impl::slot_info_t>& flux::impl::Layout::_materialize() {
    if (auto* owned = std::get_if<slot_info_array>(&_repr))
        return *owned;

    const auto fixed = std::get<slot_info_view>(_repr);

    return _repr.emplace<slot_info_array>(fixed.begin(), fixed.end());
}

ers::Status flux::impl::Layout::_insert_at(slot_ref anchor, const slot_info_t& binding, ptrdiff_t offset) {
    if (!anchor)
        return ers::make_error(EQueryError::NullSlot, "anchor");

    ERS_TRY(_check_insertable(binding.slot));

    const auto at = index_of(anchor);
    if (!at)
        return ers::make_error(EQueryError::UnknownAnchor, "'{}'", anchor->name);

    auto& owned = _materialize();
    owned.emplace(owned.begin() + static_cast<ptrdiff_t>(*at) + offset, binding);

    return ers::ok;
}

ers::Status flux::impl::Layout::insert_after(slot_ref anchor, const slot_info_t& binding) {
    return _insert_at(anchor, binding, 1);
}

ers::Status flux::impl::Layout::insert_before(slot_ref anchor, const slot_info_t& binding) {
    return _insert_at(anchor, binding, 0);
}

ers::Status flux::impl::Layout::append(slot_info_t binding) {
    ERS_TRY(_check_insertable(binding.slot));

    _materialize().push_back(binding);

    return ers::ok;
}


// Validators

ers::Status flux::impl::Layout::_check_insertable(slot_ref slot) const {
    if (!slot)
        return ers::make_error(EQueryError::NullSlot);

    if (contains(slot))
        return ers::make_error(EQueryError::DuplicateSlot, "'{}'", slot->name);

    return ers::ok;
}
