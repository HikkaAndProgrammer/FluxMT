#pragma once

// std
#include <memory>
#include <string_view>

// pqxx
#include <pqxx/connection>

// ers
#include <erslib/core/type/diagnostic.hpp>

// fluxmt
#include <fluxmt/psql/options.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::psql {
    FLUXMT_EXPORT std::unique_ptr<pqxx::connection> open_connection(const options_t& options);
    FLUXMT_EXPORT bool is_alive(const pqxx::connection& conn) noexcept;

    FLUXMT_EXPORT ers::Diagnostic diagnose(std::string_view label);
    FLUXMT_EXPORT bool retryable() noexcept;
}
