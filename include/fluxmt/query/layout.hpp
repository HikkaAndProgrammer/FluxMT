#pragma once

// std
#include <cstddef>
#include <span>
#include <variant>

// ers
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/query/slot.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class FLUXMT_EXPORT Layout {
    public:
        // Member functions

        constexpr Layout(slot_info_view fixed) noexcept :
            _repr(fixed) {
        }


        // Observers

        slot_info_view bindings() const noexcept;

        ers::maybe<size_t> index_of(slot_ref slot) const noexcept;

        bool contains(slot_ref slot) const noexcept;


        // Modifiers

        ers::Status insert_after(slot_ref anchor, const slot_info_t& binding);
        ers::Status insert_before(slot_ref anchor, const slot_info_t& binding);

        ers::Status append(slot_info_t slot);


    private:
        std::variant<slot_info_view, slot_info_array> _repr;


        slot_info_array& _materialize();

        ers::Status _check_insertable(slot_ref slot) const;
        ers::Status _insert_at(slot_ref anchor, const slot_info_t& binding, ptrdiff_t offset);
    };
}
