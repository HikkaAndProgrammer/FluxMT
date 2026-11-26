#pragma once

// std
#include <string>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/query/clause.hpp>


namespace flux::impl {
    class IdentityClause final : public IClause {
    public:
        // Member functions

        IdentityClause(SlotRef slot, std::string name, std::string alias = "") :
            IClause(slot),
            _name(std::move(name)),
            _alias(std::move(alias)) {
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            ERS_TRY(ctx.append_identifier(_name));

            if (_alias.empty())
                return ers::ok;

            ctx.query += " AS ";

            return ctx.append_identifier(_alias);
        }


    private:
        std::string _name;
        std::string _alias;
    };
}
