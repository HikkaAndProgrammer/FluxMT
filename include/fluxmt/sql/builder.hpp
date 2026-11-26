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
    template<typename B, kind_ref K>
        requires HasKind<B, K>
    class QueryBuilder {
    public:
        using backend = B;

        static constexpr kind_ref kind = K;


        // Member functions

        explicit QueryBuilder(Query query) :
            _query(std::move(query)) {
        }


        // Accessors

        const Query& erased() const & noexcept { return _query; }
        Query& erased() & noexcept { return _query; }
        Query erased() && { return std::move(_query); }

        ers::Result<statement_t> to_statement() const {
            return _query.to_statement(backend_traits<B>::dialect());
        }


        // Operators

        template<typename C>
            requires is_slot_clause_v<C>
        QueryBuilder& operator|=(C clause) {
            _check<C>();
            _query.add(std::move(clause));

            return *this;
        }

        template<typename C>
            requires is_slot_clause_v<C>
        friend QueryBuilder operator|(QueryBuilder lhs, C clause) {
            lhs |= std::move(clause);

            return lhs;
        }


    private:
        Query _query;


        template<typename C>
        static constexpr void _check() noexcept {
            static_assert(HasSlot<B, K, C::slot>, "this statement has no such clause on this backend");
            static_assert(SupportsNeeds<B, clause_needs<C>()>, "this backend lacks syntax this clause needs");
        }
    };
}
