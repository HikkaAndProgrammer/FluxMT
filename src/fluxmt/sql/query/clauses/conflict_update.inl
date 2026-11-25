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
#include <fluxmt/sql/query/clause.hpp>
#include <fluxmt/sql/query/slots/conflict.hpp>


namespace flux::impl {
    // The whole action: "DO UPDATE SET a = excluded.a, b = excluded.b"
    class ConflictUpdateClause final : public IClause {
    public:
        // Member functions

        explicit ConflictUpdateClause(std::vector<std::string> columns) :
            IClause(&slots::conflict_action),
            _columns(std::move(columns)) {
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            if (_columns.empty())
                return ers::make_error(ESqlError::EmptyColumns, "'{}'", slot()->name);

            ctx.query += "DO UPDATE SET ";

            bool first = true;

            for (const auto& name : _columns) {
                if (!first)
                    ctx.query += ", ";
                first = false;

                ERS_TRY(ctx.append_identifier(name));

                ctx.query += " = excluded.";
                ERS_TRY(ctx.append_identifier(name));
            }

            return ers::ok;
        }


    private:
        std::vector<std::string> _columns;
    };
}
