#pragma once

// std
#include <cstddef>
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/error.hpp>
#include <fluxmt/sql/expr/expr.hpp>
#include <fluxmt/sql/expr/node.hpp>


namespace flux::impl::nodes {
    class RawNode final : public IExprNode {
    public:
        // Member functions

        RawNode(std::string_view pattern, std::vector<Expr> args) :
            _pattern(pattern),
            _args(std::move(args)) {
        }


        // Observers

        EPrecedence precedence() const noexcept override { return EPrecedence::Or; }


        // Executors

        ers::Status render(Context& ctx) const override {
            std::string_view rest = _pattern;
            size_t used = 0;

            for (auto at = rest.find("{}"); at != std::string_view::npos; at = rest.find("{}")) {
                if (used == _args.size()) {
                    return ers::make_error(ESqlError::RawPlaceholderMismatch,
                        "Raw SQL '{}' has more '{{}}' than arguments.",
                        _pattern);
                }

                ctx.query += rest.substr(0, at);
                ERS_TRY(_args[used++].render(ctx, EPrecedence::Primary));

                rest.remove_prefix(at + 2);
            }

            if (used != _args.size()) {
                return ers::make_error(ESqlError::RawPlaceholderMismatch,
                    "Raw SQL '{}' has fewer '{{}}' than arguments.",
                    _pattern);
            }

            ctx.query += rest;

            return ers::ok;
        }


    private:
        std::string_view _pattern;
        std::vector<Expr> _args;
    };
}
