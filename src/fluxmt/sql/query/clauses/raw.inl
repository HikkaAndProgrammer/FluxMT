#pragma once

// std
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core/value.hpp>
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/query/clause.hpp>


namespace flux::impl {
    class RawClause final : public IClause {
    public:
        // Member functions

        RawClause(SlotRef slot, std::string sql, std::vector<Value> values) :
            IClause(slot),
            _sql(std::move(sql)),
            _values(std::move(values)) {
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            size_t next = 0;

            for (const char it : _sql) {
                if (it != '?') {
                    ctx.query += it;
                    continue;
                }

                if (next == _values.size()) {
                    return ers::make_error(ESqlError::PlaceholderMismatch,
                        "Slot '{}' has more '?' markers than values ({}).",
                        slot()->name, _values.size());
                }

                ctx.query += ctx.bind(_values[next]);

                ++next;
            }

            if (next != _values.size()) {
                return ers::make_error(ESqlError::PlaceholderMismatch,
                    "Slot '{}' has {} values for only {} '?' markers.",
                    slot()->name, _values.size(), next);
            }

            return ers::ok;
        }


    private:
        std::string _sql;
        std::vector<Value> _values;
    };
}
