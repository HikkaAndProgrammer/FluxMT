#pragma once

// std
#include <cassert>
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/query/context.hpp>
#include <fluxmt/sql/dialect.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class FLUXMT_EXPORT SqlContext final : public Context {
    public:
        // Member functions

        explicit SqlContext(const SqlDialect& dialect) noexcept;
        SqlContext(const SqlDialect&&) = delete;


        // Observers

        const SqlDialect& dialect() const noexcept override;


        // Modifiers

        ers::Status append_identifier(std::string_view name);
    };


    inline SqlContext& sql_of(Context& ctx) noexcept {
        assert(dynamic_cast<SqlContext*>(&ctx));

        return static_cast<SqlContext&>(ctx);
    }
}
