#pragma once

// std
#include <utility>

// fluxmt
#include <fluxmt/core/value.hpp>
#include <fluxmt/sql/expr/node.hpp>


namespace flux::impl::nodes {
    // A node that carries a value: ANY() looks into it when the dialect has no arrays.
    class BoundNode : public IExprNode {
    public:
        // Member functions

        explicit BoundNode(Value value) :
            _value(std::move(value)) {
        }


        // Observers

        const Value& value() const noexcept { return _value; }


    private:
        Value _value;
    };
}
