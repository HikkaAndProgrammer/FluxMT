#pragma once

// std
#include <type_traits>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/query/slot.hpp>


namespace flux::impl {
    template<slot_ref Slot, bool Many = false, auto Meta = 0>
    class SlotClause {
    public:
        static constexpr slot_ref slot = Slot;
        static constexpr bool many = Many;
        static constexpr auto meta = Meta;


        // Member functions

        SlotClause() requires Many = default;

        explicit SlotClause(Clause clause) requires (!Many) :
            _items(std::move(clause)) {
        }

        explicit SlotClause(std::vector<Clause> clauses) requires Many :
            _items(std::move(clauses)) {
        }


        // Observers

        operator Clause() && requires (!Many) {
            return std::move(_items);
        }

        operator std::vector<Clause>() && {
            return std::move(*this).take_all();
        }


        // Modifiers

        std::vector<Clause> take_all() && {
            if constexpr (Many) {
                return std::move(_items);
            } else {
                std::vector<Clause> out;
                out.push_back(std::move(_items));

                return out;
            }
        }


    private:
        std::conditional_t<Many, std::vector<Clause>, Clause> _items;
    };


    template<typename T>
    inline constexpr bool is_slot_clause_v = false;

    template<slot_ref Slot, bool Many, auto Meta>
    inline constexpr bool is_slot_clause_v<SlotClause<Slot, Many, Meta>> = true;
}
