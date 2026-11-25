#pragma once

// std
#include <cstddef>
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    enum class ECastStyle : ers::u8 {
        Postfix,  // x::type
        Function, // or CAST(x AS type)
    };

    struct dialect_caps_t {
        bool native_arrays = false;
        bool ilike = false;
        ECastStyle cast = ECastStyle::Function;
    };
}


namespace flux::impl {
    class FLUXMT_EXPORT Dialect {
    public:
        // Member functions

        explicit Dialect(dialect_caps_t caps) noexcept :
            _caps(caps) {
        }

        virtual ~Dialect() = default;


        // Accessors

        const dialect_caps_t& caps() const noexcept { return _caps; }

        virtual std::string_view name() const noexcept = 0;

        // "$1" or "?1"; the index starts from 1
        virtual std::string placeholder(size_t index) const = 0;

        virtual bool is_reserved(std::string_view word) const noexcept = 0;


    private:
        dialect_caps_t _caps;
    };
}
