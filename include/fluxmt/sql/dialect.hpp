#pragma once

// std
#include <cstddef>
#include <string>
#include <string_view>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class FLUXMT_EXPORT Dialect {
    public:
        virtual ~Dialect() = default;


        virtual std::string_view name() const noexcept = 0;

        // "$1" or "?1"; the index starts from 1
        virtual std::string placeholder(size_t index) const = 0;

        virtual bool is_reserved(std::string_view word) const noexcept = 0;
    };
}
