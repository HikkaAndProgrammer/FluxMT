#pragma once

// std
#include <memory>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/query/fwd.hpp>
#include <fluxmt/sql/query/slot.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class FLUXMT_EXPORT IClause {
    public:
        // Member functions

        explicit IClause(SlotRef slot) noexcept :
            _slot(slot) {
        }

        virtual ~IClause() = default;


        // Observers

        SlotRef slot() const noexcept { return _slot; }


        // Executors

        virtual ers::Status render(Context& ctx) const = 0;


    private:
        SlotRef _slot;
    };


    template<typename T, typename... Args>
    Clause make_clause(Args&&... args) {
        return Clause(std::in_place_type<T>, std::forward<Args>(args)...);
    }
}
