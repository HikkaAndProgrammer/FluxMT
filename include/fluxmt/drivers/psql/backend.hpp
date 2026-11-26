#pragma once

// std
#include <string_view>

// pqxx
#include <pqxx/connection>
#include <pqxx/transaction>

// fluxmt
#include <fluxmt/core/backend.hpp>
#include <fluxmt/drivers/psql/connection.hpp>
#include <fluxmt/drivers/psql/dialect.hpp>
#include <fluxmt/drivers/psql/layouts.hpp>
#include <fluxmt/drivers/psql/options.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::psql {
    struct backend {};
}


// Specializations -------------------------------------------------------------

template<>
struct flux::impl::backend_traits<flux::impl::psql::backend> {
    using connection = pqxx::connection;
    using options = psql::options_t;
    using default_tx = pqxx::work;


    static constexpr std::string_view name = "psql";
    static constexpr feature_set features = {
        EFeature::Sql,
        EFeature::Transactions,
        EFeature::Returning,
        EFeature::Upsert,
        EFeature::DistinctOn,
        EFeature::NativeArrays,
    };


    static std::unique_ptr<connection> open(const options& opts) { return psql::open_connection(opts); }
    static bool alive(const connection& conn) noexcept { return psql::is_alive(conn); }
    static ers::Diagnostic diagnose(std::string_view label) { return psql::diagnose(label); }
    static bool retryable() noexcept { return psql::retryable(); }
    static constexpr const syntax_t& syntax = psql::postgres_syntax;

    static constexpr slot_info_view layout(kind_ref kind) noexcept { return find_layout(psql::layout_table, kind); }

    static const SqlDialect& dialect() noexcept { return psql::dialect(); }
};
