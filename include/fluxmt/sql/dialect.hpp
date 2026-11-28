#pragma once

// std
#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/query/dialect.hpp>
#include <fluxmt/sql/syntax.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class FLUXMT_EXPORT SqlDialect : public Dialect {
    public:
        // Member functions

        explicit SqlDialect(const syntax_t& syntax) noexcept :
            _syntax(syntax) {
        }


        // Accessors

        const syntax_t& syntax() const noexcept { return _syntax; }

        virtual bool is_reserved(std::string_view word) const noexcept = 0;

        std::unique_ptr<Context> make_context() const final;


    private:
        syntax_t _syntax;
    };
}
