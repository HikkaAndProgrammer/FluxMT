#pragma once

// std
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/clauses/order.hpp>
#include <fluxmt/sql/slots/order.hpp>


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

        ers::Status render(Context& base) const override {
            auto& ctx = sql_of(base);

            ERS_TRY(_what.render(ctx));

            ctx.text += _order == EOrder::Desc ? " DESC" : " ASC";

            return ers::ok;
        }


    private:
        Expr _what;
        EOrder _order;
    };
}
