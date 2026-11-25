#pragma once

// std
#include <concepts>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/query/clause.hpp>
#include <fluxmt/sql/query/layout.hpp>
#include <fluxmt/sql/statement.hpp>

// export
#include <fluxmt/export.hpp>


// Query -----------------------------------------------------------------------

namespace flux::impl {
    struct WithCte;
    struct WithUnion;
}

namespace flux::impl {
    // A statement as a tree: a layout of slots and the clauses that fill them.
    // It does not know the dialect, that comes with build().
    class FLUXMT_EXPORT Query {
    public:
        // Member functions

        explicit Query(LayoutView layout) :
            _layout(layout) {
        }


        // Observers

        Layout& layout() noexcept { return _layout; }
        const Layout& layout() const noexcept { return _layout; }

        bool has(SlotRef slot) const;


        // Modifiers

        void add(Clause clause);
        void add(std::vector<Clause> clauses);

        void add(WithCte with_cte);
        void add(WithUnion with_union);


        // Executors

        ers::Status build(Context& ctx) const;

        ers::Result<Statement> to_statement(const Dialect& dialect) const;


    private:
        Layout _layout;
        std::vector<Clause> _clauses;
        std::vector<WithCte> _ctes;
        std::vector<WithUnion> _unions;


        ers::Status _render_slot(Context& ctx, const SlotBinding& binding) const;
        ers::Status _render_custom(Context& ctx, const SlotBinding& binding) const;

        ers::Status _validate(const Dialect& dialect) const;
    };
}


// Statement-level parts --------------------------------------------------------

namespace flux::impl {
    struct WithCte {
        Query definition;
        std::string name;
        bool recursive;
    };

    struct WithUnion {
        Query other;
        bool all;
    };


    namespace clauses {
        FLUXMT_EXPORT WithCte with_cte(std::string name, Query definition, bool recursive = false);

        FLUXMT_EXPORT WithUnion union_with(Query other, bool all = false);
    }
}


// Operators -------------------------------------------------------------------

namespace flux::impl {
    template<typename T>
    concept QueryPart = requires(Query& query, T part) {
        query.add(std::move(part));
    };

    template<QueryPart T>
    Query& operator|(Query& lhs, T rhs) {
        lhs.add(std::move(rhs));
        return lhs;
    }

    template<QueryPart T>
    Query&& operator|(Query&& lhs, T rhs) {
        lhs.add(std::move(rhs));
        return std::move(lhs);
    }

    template<QueryPart T>
    Query& operator|=(Query& lhs, T rhs) {
        lhs.add(std::move(rhs));
        return lhs;
    }
}


// Subqueries ------------------------------------------------------------------

namespace flux::impl {
    // open + the query + close: "EXISTS (" ... ")"
    FLUXMT_EXPORT ers::Status append_subquery(
        Context& ctx, const Query& sub,
        std::string_view open, std::string_view close = ")"
    );
}
