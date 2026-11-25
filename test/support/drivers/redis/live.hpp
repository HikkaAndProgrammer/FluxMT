#pragma once

// std
#include <string>
#include <string_view>

// fluxmt
#include <fluxmt/drivers/redis.hpp>


namespace test::live {
    // Skips the running test case when no server is configured.
    flux::redis::options_t require_options();


    // Own key prefix per test case: its keys are deleted on destruction.
    class Sandbox {
    public:
        // Member functions

        explicit Sandbox(flux::redis::pool_options_t pool_opts = {});
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
