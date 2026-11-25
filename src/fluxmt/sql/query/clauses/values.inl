#pragma once

// std
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/query/clause.hpp>
#include <fluxmt/sql/query/slots/values.hpp>


namespace flux::impl {
    class ValuesClause final : public IClause {
    public:
        // Member functions

        explicit ValuesClause(std::vector<Expr> items) :
            IClause(&slots::values),
            _items(std::move(items)) {
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            if (_items.empty())
                return ers::make_error(ESqlError::EmptyValues, "'{}'", slot()->name);

            bool first = true;

            for (const auto& it : _items) {
                if (!first)
                    ctx.query += ", ";
                first = false;

                ERS_TRY(it.render(ctx));
            }

            return ers::ok;
        }


    private:
        std::vector<Expr> _items;
    };
}
