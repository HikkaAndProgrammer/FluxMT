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


namespace flux::impl::sqlite {
    class FLUXMT_EXPORT sqlite_dialect final : public Dialect {
    public:
        sqlite_dialect() noexcept;

        std::string_view name() const noexcept override;
        std::string placeholder(size_t index) const override;
        bool is_reserved(std::string_view word) const noexcept override;

        bool supports(const Slot* slot) const noexcept override;
        ers::Status before_slot(Context& ctx, const Query& query, const SlotBinding& binding) const override;
        ers::Result<function_spelling_t> function_spelling(std::string_view name, size_t arity) const override;
    };


    FLUXMT_EXPORT const sqlite_dialect& dialect() noexcept;

    FLUXMT_EXPORT std::span<const std::string_view> reserved_words() noexcept;
}
