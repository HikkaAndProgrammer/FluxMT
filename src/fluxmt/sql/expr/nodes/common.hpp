#pragma once

// std
#include <vector>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>


namespace flux::impl::nodes {
    constexpr EPrecedence next(EPrecedence precedence) noexcept {
        return precedence == EPrecedence::Primary
            ? precedence
            : static_cast<EPrecedence>(static_cast<ers::u8>(precedence) + 1);
    }

    inline ers::Status render_list(SqlContext& ctx, const std::vector<Expr>& items) {
        bool first = true;

        for (const auto& it : items) {
            if (!first)
                ctx.text += ", ";
            first = false;

            ERS_TRY(it.render(ctx));
        }

        return ers::ok;
    }
}
