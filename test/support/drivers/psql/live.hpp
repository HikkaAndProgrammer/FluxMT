#pragma once

// std
#include <string>
#include <utility>

// fluxmt
#include <fluxmt/drivers/psql.hpp>


namespace test::live {
    flux::psql::options_t require_options();


    class Sandbox {
    public:
        // Member functions

        explicit Sandbox(flux::pool_options_t pool_opts = {});
        ~Sandbox();

        Sandbox(const Sandbox&) = delete;
        Sandbox& operator=(const Sandbox&) = delete;


        // Accessors

        flux::psql::Database& db() noexcept { return _db; }
        const std::string& schema() const noexcept { return _schema; }

        [[nodiscard]] std::string sql(std::string text) const;

        template<typename T>
        T scalar(std::string query) {
            return flux::with_tx(_db, [&](pqxx::work& tx) {
                return tx.query_value<T>(sql(std::move(query)));
            }, "scalar");
        }


    private:
        flux::psql::Database _db;
        std::string _schema;
    };
}
