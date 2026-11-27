#pragma once

// std
#include <memory>
#include <string_view>

// fluxmt
#include <fluxmt/core/backend.hpp>
#include <fluxmt/drivers/sqlite/connection.hpp>
#include <fluxmt/drivers/sqlite/dialect.hpp>
#include <fluxmt/drivers/sqlite/error.hpp>
#include <fluxmt/drivers/sqlite/options.hpp>
#include <fluxmt/drivers/sqlite/transaction.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::sqlite {
    struct backend {};
}


// Specializations -------------------------------------------------------------

template<>
struct flux::ext::backend_traits<flux::impl::sqlite::backend> {
    using connection = impl::sqlite::connection;
    using options = impl::sqlite::options_t;
    using default_tx = impl::sqlite::transaction;


    static constexpr std::string_view name = "sqlite";
    static constexpr feature_set features = {
        EFeature::Sql,
        EFeature::Transactions,
        EFeature::Returning,
        EFeature::Upsert,
    };


    static std::unique_ptr<connection> open(const options& opts) { return std::make_unique<connection>(opts); }
    static bool alive(const connection& conn) noexcept { return conn.is_open(); }
    static ers::Diagnostic diagnose(std::string_view label) { return impl::sqlite::diagnose(label); }
    static bool retryable() noexcept { return impl::sqlite::retryable(); }
    static constexpr const impl::syntax_t& syntax = impl::sqlite::sqlite_syntax;

    static const impl::SqlDialect& dialect() noexcept { return impl::sqlite::dialect(); }
};
