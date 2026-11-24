#pragma once

// std
#include <memory>
#include <string_view>

// fluxmt
#include <fluxmt/core/backend.hpp>
#include <fluxmt/drivers/sqlite/connection.hpp>
#include <fluxmt/drivers/sqlite/error.hpp>
#include <fluxmt/drivers/sqlite/options.hpp>
#include <fluxmt/drivers/sqlite/transaction.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::sqlite {
    struct backend {};
}


// Specializations -------------------------------------------------------------

template<>
struct flux::impl::backend_traits<flux::impl::sqlite::backend> {
    using connection = sqlite::connection;
    using options = sqlite::options_t;
    using default_tx = sqlite::transaction;


    static constexpr std::string_view name = "sqlite";
    static constexpr feature_set features = {
        EFeature::Sql,
        EFeature::Transactions,
        EFeature::Returning,
        EFeature::Upsert,
    };


    static std::unique_ptr<connection> open(const options& opts) { return std::make_unique<connection>(opts); }
    static bool alive(const connection& conn) noexcept { return conn.is_open(); }
    static ers::Diagnostic diagnose(std::string_view label) { return sqlite::diagnose(label); }
    static bool retryable() noexcept { return sqlite::retryable(); }
};
