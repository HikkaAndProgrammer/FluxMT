#pragma once

// std
#include <memory>
#include <string_view>

// fluxmt
#include <fluxmt/core/backend.hpp>
#include <fluxmt/drivers/redis/connection.hpp>
#include <fluxmt/drivers/redis/error.hpp>
#include <fluxmt/drivers/redis/options.hpp>
#include <fluxmt/drivers/redis/transaction.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::redis {
    struct backend {};
}


// Specializations -------------------------------------------------------------

template<>
struct flux::ext::backend_traits<flux::impl::redis::backend> {
    using connection = impl::redis::connection;
    using options = impl::redis::options_t;
    using default_tx = impl::redis::transaction;


    static constexpr std::string_view name = "redis";
    static constexpr feature_set features = {
        EFeature::Transactions,
    };


    static std::unique_ptr<connection> open(const options& opts) { return std::make_unique<connection>(opts); }
    static bool alive(const connection& conn) noexcept { return conn.is_open(); }
    static ers::Diagnostic diagnose(std::string_view label) { return impl::redis::diagnose(label); }
    static bool retryable() noexcept { return impl::redis::retryable(); }
};
