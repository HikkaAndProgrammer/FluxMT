#pragma once

// std
#include <string_view>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/expr/aggregate.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>
#include <fluxmt/sql/identifier.hpp>

// nodes
#include "../common.hpp"


namespace flux::impl::nodes {
    class AggregateNode final : public IExprNode {
    public:
        // Member functions

        AggregateNode(std::string_view name, aggregate_t what) :
            _name(name),
            _what(std::move(what)) {
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            if (!is_valid_name(_name)) {
                return ers::make_error(ESqlError::InvalidName, "aggregate '{}'",
                    _name);
            }

            if (_what.ordered_set && _what.within_group.empty()) {
                return ers::make_error(ESqlError::MissingOrdering, "'{}' needs WITHIN GROUP",
                    _name);
            }

            ctx.query += _name;
            ctx.query += '(';

            if (_what.distinct)
                ctx.query += "DISTINCT ";

            ERS_TRY(render_list(ctx, _what.args));

            if (!_what.order_by.empty()) {
                ctx.query += " ORDER BY ";
                ERS_TRY(render_list(ctx, _what.order_by));
            }

            ctx.query += ')';

            if (_what.ordered_set) {
                ctx.query += " WITHIN GROUP (ORDER BY ";
                ERS_TRY(render_list(ctx, _what.within_group));
                ctx.query += ')';
            }

            return ers::ok;
        }


    private:
        std::string_view _name;
        aggregate_t _what;
    };
}
