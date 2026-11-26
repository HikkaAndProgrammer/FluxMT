#pragma once

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>

// nodes
#include "bound.hpp"


namespace flux::impl::nodes {
    class ValueNode final : public BoundNode {
    public:
        using BoundNode::BoundNode;


        // Executors

        ers::Status render(SqlContext& ctx) const override {
            ctx.text += ctx.bind(value());

            return ers::ok;
        }
    };
}
