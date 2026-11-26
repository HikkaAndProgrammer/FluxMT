#pragma once

// std
#include <string>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/slots/set.hpp>


namespace flux::impl {
    class AssignClause final : public IClause {
    public:
        // Member functions

        AssignClause(std::string column, Expr value) :
            IClause(&slots::set),
            _column(std::move(column)),
            _value(std::move(value)) {
        }


        // Executors

        ers::Status render(Context& base) const override {
            auto& ctx = sql_of(base);

            ERS_TRY(render_column(ctx));

            ctx.text += " = ";

            return render_value(ctx);
        }

        ers::Status render_column(SqlContext& ctx) const {
            return ctx.append_identifier(_column);
        }

        ers::Status render_value(SqlContext& ctx) const {
            return _value.render(ctx);
        }


    private:
        std::string _column;
        Expr _value;
    };
}
