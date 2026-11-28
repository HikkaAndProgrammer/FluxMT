#pragma once

// std
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/query/query.hpp>
#include <fluxmt/query/slot_clause.hpp>
#include <fluxmt/query/statement.hpp>
#include <fluxmt/sql/backend.hpp>
#include <fluxmt/sql/clauses/tag.hpp>


namespace flux::impl {
    template<IsSqlBackend Backend, const auto& Layout>
    class QueryBuilder {
    public:
        using backend = Backend;

        static constexpr slot_info_view layout = Layout;


        // Member functions

        QueryBuilder() :
            _query(layout) {
        }


        // Accessors

        const Query& erased() const & noexcept { return _query; }
        Query& erased() & noexcept { return _query; }
        Query erased() && { return std::move(_query); }

        ers::Result<statement_t> to_statement() const {
            return _query.to_statement(ext::backend_traits<Backend>::dialect());
        }


        // Conversions

        operator Query() const & { return _query; }
        operator Query() && { return std::move(_query); }


        // Operators

        template<typename Tag>
            requires is_slot_clause_v<Tag> && AcceptsClause<Backend, Layout, Tag>
        QueryBuilder& operator|=(Tag clause) {
            _check<Tag>();
            _query.add(std::move(clause));

            return *this;
        }

        template<typename Tag>
            requires is_slot_clause_v<Tag> && AcceptsClause<Backend, Layout, Tag>
        friend QueryBuilder operator|(QueryBuilder lhs, Tag clause) {
            lhs |= std::move(clause);

            return lhs;
        }


    private:
        Query _query;


        template<typename Tag>
        static constexpr void _check() noexcept {
            static_assert(HasSlot<Backend, Layout, Tag::slot>, "this statement has no such clause on this backend");
            static_assert(SupportsNeeds<Backend, clause_needs<Tag>()>, "this backend lacks syntax this clause needs");
        }
    };
}
