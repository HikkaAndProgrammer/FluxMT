#pragma once

// std
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/query/clause.hpp>


namespace flux::impl {
    class ConditionClause final : public IClause {
    public:
        // Member functions

        ConditionClause(slot_ref slot, Expr condition) :
            IClause(slot),
            _condition(std::move(condition)) {
        }


        // Executors

        ers::Status render(Context& base) const override {
            auto& ctx = sql_of(base);

            return _condition.render(ctx, EPrecedence::And);
        }


    private:
        Expr _condition;
    };
}
