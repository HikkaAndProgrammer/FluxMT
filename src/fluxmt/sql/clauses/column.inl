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


namespace flux::impl {
    class ColumnClause final : public IClause {
    public:
        // Member functions

        ColumnClause(slot_ref slot, Expr what, std::string alias = "") :
            IClause(slot),
            _what(std::move(what)),
            _alias(std::move(alias)) {
        }


        // Executors

        ers::Status render(Context& base) const override {
            auto& ctx = sql_of(base);

            ERS_TRY(_what.render(ctx));

            if (_alias.empty())
                return ers::ok;

            ctx.text += " AS ";

            return ctx.append_identifier(_alias);
        }


    private:
        Expr _what;
        std::string _alias;
    };
}
