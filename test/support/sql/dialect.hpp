#pragma once

// std
#include <cstddef>
#include <string>
#include <string_view>

// fluxmt
#include <fluxmt/sql.hpp>


namespace test {
    // Differs from PostgreSQL on purpose: "?N" placeholders and its own reserved words.
    class AntiDialect final : public flux::sql::Dialect {
    public:
        AntiDialect() noexcept : Dialect(flux::sql::dialect_caps_t {}) {}

        std::string_view name() const noexcept override { return "anti"; }
        std::string placeholder(size_t index) const override { return "?" + std::to_string(index); }
        bool is_reserved(std::string_view word) const noexcept override { return word == "item" || word == "group"; }
    };
}
