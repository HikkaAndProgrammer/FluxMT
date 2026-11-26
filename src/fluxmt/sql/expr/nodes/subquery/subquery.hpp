#pragma once

// std
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/node.hpp>
#include <fluxmt/sql/query/query.hpp>


namespace flux::impl::nodes {
    class SubqueryNode final : public IExprNode {
    public:
        // Member functions

        SubqueryNode(Query query, bool exists) :
            _query(std::move(query)),
            _exists(exists) {
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            return append_subquery(ctx, _query, _exists ? "EXISTS (" : "(");
        }


    private:
        Query _query;
        bool _exists;
    };
}
