#pragma once

// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <cstdio>
#include <cstdlib>
#include <format>
#include <print>
#include <random>
#include <string>
#include <utility>

// ers
#include <erslib/core/convert/number.hpp>

// fluxmt
#include <fluxmt/psql.hpp>


#ifndef FLUXMT_TEST_PG_HOST_DEFAULT
#define FLUXMT_TEST_PG_HOST_DEFAULT ""
#define FLUXMT_TEST_PG_PORT_DEFAULT ""
#define FLUXMT_TEST_PG_DB_DEFAULT ""
#define FLUXMT_TEST_PG_USER_DEFAULT ""
#define FLUXMT_TEST_PG_PASSWORD_DEFAULT ""
#endif


namespace test::live {
    inline std::string setting(const char* name, const char* compiled) {
        const char* value = std::getenv(name);
        return value && *value ? value : compiled;
    }

    inline flux::psql::options_t require_options() {
        auto dbname = setting("FLUXMT_TEST_PG_DB", FLUXMT_TEST_PG_DB_DEFAULT);
        if (dbname.empty())
            SKIP("FLUXMT_TEST_PG_DB is not set");

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

    inline std::string make_schema_name() {
        std::random_device rd;
        return std::format("fluxmt_t_{:08x}{:08x}", rd(), rd());
    }


    class Sandbox {
    public:
        explicit Sandbox(flux::psql::pool_options_t pool_opts = {}) :
            _db(require_options(), pool_opts),
            _schema(make_schema_name()) {
            flux::psql::with_tx(_db, [&](pqxx::work& tx) {
                tx.exec(std::format("CREATE SCHEMA {}", _schema));
            }, "sandbox create");
        }

        Sandbox(const Sandbox&) = delete;
        Sandbox& operator=(const Sandbox&) = delete;

        ~Sandbox() {
            auto dropped = flux::psql::try_with_tx(_db, [&](pqxx::work& tx) {
                tx.exec(std::format("DROP SCHEMA IF EXISTS {} CASCADE", _schema));
            }, "sandbox drop");

            if (!dropped) {
                std::println(stderr, "sandbox {} was not dropped: {}\n",
                    _schema.c_str(), dropped.error().message());
            }
        }


        flux::psql::Database& db() noexcept { return _db; }
        const std::string& schema() const noexcept { return _schema; }

        // '@' in the text stands for the schema name: "SELECT * FROM @.account"
        std::string sql(std::string text) const {
            for (size_t pos = 0; (pos = text.find('@', pos)) != std::string::npos; pos += _schema.size())
                text.replace(pos, 1, _schema);

            return text;
        }

        template<typename T>
        T scalar(std::string query) {
            return flux::psql::with_tx(_db, [&](pqxx::work& tx) {
                return tx.query_value<T>(sql(std::move(query)));
            }, "scalar");
        }


    private:
        flux::psql::Database _db;
        std::string _schema;
    };
}
