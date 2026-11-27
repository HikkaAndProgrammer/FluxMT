#pragma once

// std
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <string_view>

// fluxmt
#include <fluxmt/query/slot.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class Context;
}


namespace flux::impl {
    class FLUXMT_EXPORT Dialect {
    public:
        // Member functions

        virtual ~Dialect() = default;


        // Accessors

        virtual std::string_view name() const noexcept = 0;
        virtual std::string placeholder(size_t index) const = 0;

        virtual std::unique_ptr<Context> make_context() const = 0;
    };
}
