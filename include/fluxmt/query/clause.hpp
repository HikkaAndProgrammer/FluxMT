#pragma once

// std
#include <memory>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/query/context.hpp>
#include <fluxmt/query/fwd.hpp>
#include <fluxmt/query/slot.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class FLUXMT_EXPORT IClause {
    public:
        // Member functions

        explicit IClause(slot_ref slot) noexcept :
            _slot(slot) {
        }

        virtual ~IClause() = default;


        // Observers

        slot_ref slot() const noexcept { return _slot; }


        // Executors

        virtual ers::Status render(Context& ctx) const = 0;


    private:
        slot_ref _slot;
    };


    template<typename T, typename... Args>
    Clause make_clause(Args&&... args) {
        return Clause(std::in_place_type<T>, std::forward<Args>(args)...);
    }
}
