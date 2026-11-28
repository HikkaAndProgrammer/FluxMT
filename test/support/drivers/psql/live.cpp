#include "support/drivers/psql/live.hpp"

// support
#include "support/livedb.hpp"

// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <cstdlib>
#include <format>
#include <print>
#include <random>

// ers
#include <erslib/core/convert/number.hpp>


#ifndef FLUXMT_TEST_PG_HOST_DEFAULT
#define FLUXMT_TEST_PG_HOST_DEFAULT ""
#define FLUXMT_TEST_PG_PORT_DEFAULT ""
#define FLUXMT_TEST_PG_DB_DEFAULT ""
#define FLUXMT_TEST_PG_USER_DEFAULT ""
#define FLUXMT_TEST_PG_PASSWORD_DEFAULT ""
#endif


// Details ---------------------------------------------------------------------

namespace {
    std::string setting(const char* name, const char* compiled) {
        const char* value = std::getenv(name);
        return value && *value ? value : compiled;
    }

    std::string make_schema_name() {
        std::random_device rd;
        return std::format("fluxmt_t_{:08x}{:08x}", rd(), rd());
    }
}


// Implementation --------------------------------------------------------------

flux::psql::options_t test::live::require_options() {
    auto dbname = setting("FLUXMT_TEST_PG_DB", FLUXMT_TEST_PG_DB_DEFAULT);
    if (dbname.empty())
        SKIP("FLUXMT_TEST_PG_DB is not set");

    test::live::start_database("pg");


    flux::psql::options_t opts;
    
    opts.dbname = std::move(dbname);

    if (auto host = setting("FLUXMT_TEST_PG_HOST", FLUXMT_TEST_PG_HOST_DEFAULT); !host.empty())
        opts.host = std::move(host);
    if (auto port = setting("FLUXMT_TEST_PG_PORT", FLUXMT_TEST_PG_PORT_DEFAULT); !port.empty())
        opts.port = static_cast<ers::u16>(std::stoi(port));

    opts.username = setting("FLUXMT_TEST_PG_USER", FLUXMT_TEST_PG_USER_DEFAULT);
    opts.password = setting("FLUXMT_TEST_PG_PASSWORD", FLUXMT_TEST_PG_PASSWORD_DEFAULT);

    return opts;
}

test::live::Sandbox::Sandbox(flux::pool_options_t pool_opts) :
    _db(require_options(), pool_opts),
    _schema(make_schema_name()) {
    flux::with_tx(_db, [&](pqxx::work& tx) {
        tx.exec(std::format("CREATE SCHEMA {}", _schema));
    }, "sandbox create");
}

test::live::Sandbox::~Sandbox() {
    auto dropped = flux::try_with_tx(_db, [&](pqxx::work& tx) {
        tx.exec(std::format("DROP SCHEMA IF EXISTS {} CASCADE", _schema));
    }, "sandbox drop");

    if (!dropped)
        std::println(stderr, "sandbox {} was not dropped: {}", _schema, dropped.error().message());
}

std::string test::live::Sandbox::sql(std::string text) const {
    for (size_t pos = 0; (pos = text.find('@', pos)) != std::string::npos; pos += _schema.size())
        text.replace(pos, 1, _schema);

    return text;
}
