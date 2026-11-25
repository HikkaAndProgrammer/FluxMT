#pragma once

// std
#include <cstddef>
#include <span>
#include <variant>
#include <vector>

// ers
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/query/slot.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class FLUXMT_EXPORT Layout {
        using LayoutArray = std::vector<SlotBinding>;

    public:
        // Member functions

        constexpr Layout(LayoutView fixed) noexcept :
            _repr(fixed) {
        }


        // Observers

        LayoutView bindings() const noexcept;

        ers::maybe<size_t> index_of(SlotRef slot) const noexcept;

        bool contains(SlotRef slot) const noexcept;


        // Modifiers

        ers::Status insert_after(SlotRef anchor, const SlotBinding& binding);
        ers::Status insert_before(SlotRef anchor, const SlotBinding& binding);

        ers::Status append(SlotBinding slot);


    private:
        std::variant<LayoutView, LayoutArray> _repr;


        LayoutArray& _materialize();

        ers::Status _check_insertable(SlotRef slot) const;
        ers::Status _insert_at(SlotRef anchor, const SlotBinding& binding, ptrdiff_t offset);
    };
}
