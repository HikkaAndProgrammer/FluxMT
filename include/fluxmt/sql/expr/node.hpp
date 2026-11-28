#pragma once

// std
#include <memory>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/context.hpp>

// export
#include <fluxmt/export.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    enum class EPrecedence : ers::u8 {
        Or, And,
        Not, Is,
        Comparison, Like,
        Other,
        AddSub, MulDiv, Exponent,
        At, Collate,
        Unary,
        Subscript,
        Cast,
        Primary
    };
}


namespace flux::impl {
    class FLUXMT_EXPORT IExprNode {
    public:
        // Member functions

        virtual ~IExprNode() = default;


        // Observers

        virtual EPrecedence precedence() const noexcept {
            return EPrecedence::Primary;
        }


        // Executors

        virtual ers::Status render(SqlContext& ctx) const = 0;
    };
}
