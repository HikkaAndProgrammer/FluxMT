#pragma once

// std
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/query/clause.hpp>
#include <fluxmt/sql/slots/conflict.hpp>


namespace flux::impl {
    class ConflictUpdateClause final : public IClause {
    public:
        // Member functions

        explicit ConflictUpdateClause(std::vector<std::string> columns) :
            IClause(&slots::conflict_action),
            _columns(std::move(columns)) {
        }


        // Executors

        ers::Status render(Context& base) const override {
            auto& ctx = sql_of(base);

            if (_columns.empty())
                return ers::make_error(ESqlError::EmptyColumns, "'{}'", slot()->name);

            const auto excluded = ctx.dialect().syntax().excluded;

            if (!excluded) {
                return ers::make_error(EQueryError::Unsupported, "DO UPDATE in '{}'",
                    ctx.dialect().name());
            }

            ctx.text += "DO UPDATE SET ";

            bool first = true;

            for (const auto& name : _columns) {
                if (!first)
                    ctx.text += ", ";
                first = false;

                ERS_TRY(ctx.append_identifier(name));

                ctx.text += " = ";
                ERS_TRY(excluded(ctx, name));
            }

            return ers::ok;
        }


    private:
        std::vector<std::string> _columns;
    };
}
