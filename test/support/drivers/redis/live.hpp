#pragma once

// std
#include <string>
#include <string_view>

// fluxmt
#include <fluxmt/drivers/redis.hpp>


namespace test::live {
    flux::redis::options_t require_options();


    class Sandbox {
    public:
        // Member functions

        explicit Sandbox(flux::pool_options_t pool_opts = {});
        ~Sandbox();

        Sandbox(const Sandbox&) = delete;
        Sandbox& operator=(const Sandbox&) = delete;


        // Accessors

        flux::redis::Database& db() noexcept { return _db; }

        std::string key(std::string_view name) const;


    private:
        flux::redis::Database _db;
        std::string _prefix;
    };
}
