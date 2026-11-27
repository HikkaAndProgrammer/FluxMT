#pragma once

// std
#include <concepts>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/query/clause.hpp>
#include <fluxmt/query/context.hpp>
#include <fluxmt/query/dialect.hpp>
#include <fluxmt/query/layout.hpp>
#include <fluxmt/query/slot_clause.hpp>
#include <fluxmt/query/statement.hpp>

// export
#include <fluxmt/export.hpp>


// Query -----------------------------------------------------------------------

namespace flux::impl {
    class FLUXMT_EXPORT Query {
    public:
        // Member functions

        explicit Query(slot_info_view layout) noexcept :
            _fixed(layout) {
        }


        // Observers

        bool has(slot_ref slot) const;

        ers::Result<Layout> layout() const;


        // Modifiers

        void add(Clause clause);
        void add(std::vector<Clause> clauses);

        template<slot_ref S, bool Many, auto Meta>
        void add(SlotClause<S, Many, Meta> clause) {
            add(std::move(clause).take_all());
        }

        ers::Status insert_after(slot_ref anchor, const slot_info_t& binding);
        ers::Status insert_before(slot_ref anchor, const slot_info_t& binding);
        ers::Status append(const slot_info_t& binding);


        // Executors

        ers::Status build(Context& ctx) const;

        ers::Result<statement_t> to_statement(const Dialect& dialect) const;


    private:
        enum class EEdit : ers::u8 {
            After, Before, Append
        };

        struct layout_edit_t {
            EEdit op;
            slot_ref anchor;
            slot_info_t binding;
        };


        slot_info_view _fixed;
        std::vector<layout_edit_t> _edits;
        std::vector<Clause> _clauses;


        ers::Status _add_edit(EEdit op, slot_ref anchor, const slot_info_t& binding);

        ers::Status _render_slot(Context& ctx, const slot_info_t& binding) const;
        ers::Status _render_custom(Context& ctx, const slot_info_t& binding) const;

        ers::Status _validate(const Layout& layout, const Dialect& dialect) const;
    };
}


// Operators -------------------------------------------------------------------

namespace flux::impl {
    template<typename T>
    concept IsQueryPart = requires(Query& query, T part) {
        query.add(std::move(part));
    };

    template<IsQueryPart T>
    Query& operator|(Query& lhs, T rhs) {
        lhs.add(std::move(rhs));
        return lhs;
    }

    template<IsQueryPart T>
    Query&& operator|(Query&& lhs, T rhs) {
        lhs.add(std::move(rhs));
        return std::move(lhs);
    }

    template<IsQueryPart T>
    Query& operator|=(Query& lhs, T rhs) {
        lhs.add(std::move(rhs));
        return lhs;
    }
}


// Subqueries ------------------------------------------------------------------

namespace flux::impl {
    FLUXMT_EXPORT ers::Status append_subquery(
        Context& ctx, const Query& sub,
        std::string_view open, std::string_view close = ")"
    );
}
