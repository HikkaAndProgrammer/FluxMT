#pragma once

// std
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/sql/query/kind.hpp>
#include <fluxmt/sql/query/slot.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class Context;
    class Expr;
}


namespace flux::impl {
    struct function_spelling_t {
        std::string_view name;
        bool call = true;

        bool supported = true;
        std::string_view reason;
    };


    struct syntax_t {
        ers::Status (*cast)(Context&, const Expr& operand, std::string_view type) = nullptr;

        ers::Status (*ilike)(Context&, const Expr& lhs, const Expr& rhs) = nullptr;

        ers::Status (*any)(Context&, const Expr& operand, const Expr& array) = nullptr;

        ers::Status (*excluded)(Context&, std::string_view column) = nullptr;

        function_spelling_t (*function)(std::string_view name, size_t arity) noexcept = nullptr;
    };


    struct layout_entry_t {
        KindRef kind;
        LayoutView layout;
    };

    constexpr LayoutView find_layout(std::span<const layout_entry_t> table, KindRef kind) noexcept {
        for (const auto& it : table) {
            if (same_kind(it.kind, kind))
                return it.layout;
        }

        return {};
    }
}


namespace flux::impl {
    class FLUXMT_EXPORT Dialect {
    public:
        // Member functions

        explicit Dialect(const syntax_t& syntax) noexcept :
            _syntax(syntax) {
        }

        virtual ~Dialect() = default;


        // Accessors

        const syntax_t& syntax() const noexcept { return _syntax; }

        virtual std::string_view name() const noexcept = 0;

        virtual std::string placeholder(size_t index) const = 0;

        virtual bool is_reserved(std::string_view word) const noexcept = 0;

        virtual LayoutView layout(KindRef kind) const noexcept = 0;


    private:
        syntax_t _syntax;
    };
}
