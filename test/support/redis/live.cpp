#include "support/redis/live.hpp"

// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <cstdlib>
#include <format>
#include <print>
#include <random>
#include <vector>


#ifndef FLUXMT_TEST_REDIS_HOST_DEFAULT
#define FLUXMT_TEST_REDIS_HOST_DEFAULT ""
#define FLUXMT_TEST_REDIS_PORT_DEFAULT ""
#define FLUXMT_TEST_REDIS_DB_DEFAULT ""
#endif


// Details ---------------------------------------------------------------------

namespace {
    std::string setting(const char* name, const char* compiled) {
        const char* value = std::getenv(name);
        return value && *value ? value : compiled;
    }

    std::string make_prefix() {
        std::random_device rd;
        return std::format("fluxmt:t:{:08x}{:08x}:", rd(), rd());
    }
}


// Implementation --------------------------------------------------------------

flux::redis::options_t test::live::require_options() {
    auto host = setting("FLUXMT_TEST_REDIS_HOST", FLUXMT_TEST_REDIS_HOST_DEFAULT);
    if (host.empty())
        SKIP("FLUXMT_TEST_REDIS_HOST is not set");


    flux::redis::options_t opts;

    opts.host = std::move(host);

    if (auto port = setting("FLUXMT_TEST_REDIS_PORT", FLUXMT_TEST_REDIS_PORT_DEFAULT); !port.empty())
        opts.port = static_cast<ers::u16>(std::stoi(port));
    if (auto db = setting("FLUXMT_TEST_REDIS_DB", FLUXMT_TEST_REDIS_DB_DEFAULT); !db.empty())
        opts.db = std::stoi(db);

    return opts;
}

test::live::Sandbox::Sandbox(flux::redis::pool_options_t pool_opts) :
    _db(require_options(), pool_opts),
    _prefix(make_prefix()) {
}

test::live::Sandbox::~Sandbox() {
    auto cleaned = flux::redis::try_with_conn(_db, [&](flux::redis::connection& conn) {
        std::string cursor = "0";

        do {
            auto reply = conn.exec({ "SCAN", cursor, "MATCH", _prefix + "*", "COUNT", 100 });
            const auto& parts = *reply.get_if<flux::value_array_t>();

            cursor = *parts.at(0).get_if<std::string>();

            const auto& keys = *parts.at(1).get_if<flux::value_array_t>();
            if (keys.empty())
                continue;

            std::vector<flux::Value> command { "DEL" };
            command.insert(command.end(), keys.begin(), keys.end());
            conn.exec(command);
        } while (cursor != "0");
    });

    if (!cleaned) {
        std::println(stderr, "sandbox {} was not cleaned: {}",
            _prefix, cleaned.error().message());
    }
}

std::string test::live::Sandbox::key(std::string_view name) const {
    return _prefix + std::string(name);
}
