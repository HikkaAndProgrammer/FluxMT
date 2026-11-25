#pragma once

// std
#include <cstddef>
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class Context;
    class Query;
    struct Slot;
    struct SlotBinding;
}


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

    struct function_spelling_t {
        // Empty: the function keeps its own name.
        std::string_view name;

        // False: written as a bare keyword, like CURRENT_TIMESTAMP.
        bool call = true;
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


        // Syntax hooks: the defaults write PostgreSQL-like SQL.

        // A statement with a clause in an unsupported slot fails with Unsupported.
        virtual bool supports(const Slot*) const noexcept { return true; }

        // Runs before a slot is rendered and may write text in front of it.
        virtual ers::Status before_slot(Context&, const Query&, const SlotBinding&) const { return ers::ok; }

        virtual ers::Result<function_spelling_t> function_spelling(std::string_view, size_t) const {
            return function_spelling_t {};
        }


    private:
        dialect_caps_t _caps;
    };
}
