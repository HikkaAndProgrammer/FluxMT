#pragma once

// std
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/query/clause.hpp>
#include <fluxmt/sql/query/clauses/order.hpp>
#include <fluxmt/sql/query/slots/order.hpp>


namespace flux::impl {
    class OrderClause final : public IClause {
    public:
        // Member functions

        OrderClause(Expr what, EOrder order) :
            IClause(&slots::order),
            _what(std::move(what)),
            _order(order) {
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            ERS_TRY(_what.render(ctx));

            ctx.query += _order == EOrder::Desc ? " DESC" : " ASC";

            return ers::ok;
        }


    private:
        Expr _what;
        EOrder _order;
    };
}
