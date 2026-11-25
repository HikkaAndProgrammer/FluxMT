#pragma once

// std
#include <string>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/expr/node.hpp>


namespace flux::impl::nodes {
    class ColumnNode final : public IExprNode {
    public:
        // Member functions

        explicit ColumnNode(std::string name) :
            _name(std::move(name)) {
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            return ctx.append_identifier(_name);
        }


    private:
        std::string _name;
    };
}
