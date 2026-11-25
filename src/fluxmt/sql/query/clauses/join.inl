#pragma once

// std
#include <string>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/query/clause.hpp>
#include <fluxmt/sql/query/slots/join.hpp>


namespace flux::impl {
    class JoinClause final : public IClause {
    public:
        // Member functions

        JoinClause(std::string table, std::string alias, Expr on) :
            IClause(&slots::join),
            _table(std::move(table)),
            _alias(std::move(alias)),
            _on(std::move(on)) {
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            ctx.query += "\nJOIN ";
            ERS_TRY(ctx.append_identifier(_table));

            if (!_alias.empty()) {
                ctx.query += " AS ";
                ERS_TRY(ctx.append_identifier(_alias));
            }

            ctx.query += " ON ";

            return _on.render(ctx);
        }


    private:
        std::string _table;
        std::string _alias;
        Expr _on;
    };
}
