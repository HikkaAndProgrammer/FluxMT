#pragma once

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core/value.hpp>
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/query/clause.hpp>


namespace flux::impl {
    class BoundClause final : public IClause {
    public:
        // Member functions

        BoundClause(slot_ref slot, ers::i64 amount) :
            IClause(slot),
            _amount(amount) {
        }


        // Executors

        ers::Status render(Context& base) const override {
            auto& ctx = sql_of(base);

            if (_amount < 0)
                return ers::make_error(ESqlError::NegativeBound, "'{}'", _amount);

            ctx.text += ctx.bind(Value(_amount));

            return ers::ok;
        }


    private:
        ers::i64 _amount;
    };
}
