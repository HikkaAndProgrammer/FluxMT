#include "fluxmt/drivers/psql/connection.hpp"

// std
#include <exception>

// ers
#include <erslib/core/util/string.hpp>

// fluxmt
#include <fluxmt/core/error.hpp>
#include <fluxmt/drivers/psql/error.hpp>


std::unique_ptr<pqxx::connection> flux::impl::psql::open_connection(const options_t& options) {
    return std::make_unique<pqxx::connection>(options.connection_string());
}

bool flux::impl::psql::is_alive(const pqxx::connection& conn) noexcept {
    return conn.is_open();
}

ers::Diagnostic flux::impl::psql::diagnose(std::string_view label) try {
    throw;
} catch (const pqxx::broken_connection& e) {
    return ers::make_crit(EDbError::ConnectionLost, "'{}': connection lost: {}",
        label, ers::util::trim(e.what()));
} catch (const pqxx::sql_error& e) {
    return ers::make_error(make_error_code(classify(e.sqlstate())), "'{}' failed [{}]: {}",
        label, e.sqlstate(), ers::util::trim(e.what()));
} catch (const std::exception& e) {
    return ers::make_error(EDbError::Other, "'{}' failed: {}",
        label, e.what());
} catch (...) {
    return ers::make_error(EDbError::Other, "'{}' failed: unknown exception",
        label);
}

bool flux::impl::psql::retryable() noexcept try {
    throw;
} catch (const pqxx::sql_error& e) {
    return is_retryable(classify(e.sqlstate()));
} catch (...) {
    return false;
}
