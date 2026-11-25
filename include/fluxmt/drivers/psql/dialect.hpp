#pragma once

// std
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

// fluxmt
#include <fluxmt/sql/dialect.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::psql {
    class FLUXMT_EXPORT postgres_dialect final : public Dialect {
    public:
        postgres_dialect() noexcept;

        std::string_view name() const noexcept override;
        std::string placeholder(size_t index) const override;
        bool is_reserved(std::string_view word) const noexcept override;
    };


    FLUXMT_EXPORT const postgres_dialect& dialect() noexcept;

    FLUXMT_EXPORT std::span<const std::string_view> reserved_words() noexcept;
}
