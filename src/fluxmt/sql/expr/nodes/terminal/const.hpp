#pragma once

// std
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/node.hpp>


namespace flux::impl::nodes {
    class ConstNode final : public IExprNode {
    public:
        // Member functions

        explicit ConstNode(std::string_view text) :
            _text(text) {
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            ctx.query += _text;

            return ers::ok;
        }


    private:
        std::string_view _text;
    };
}
