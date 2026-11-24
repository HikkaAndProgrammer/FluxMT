#pragma once

// std
#include <initializer_list>
#include <span>

// fluxmt
#include <fluxmt/core/value.hpp>
#include <fluxmt/drivers/redis/options.hpp>

// export
#include <fluxmt/export.hpp>


struct redisContext;


namespace flux::impl::redis {
    class FLUXMT_EXPORT connection {
    public:
        // Member functions

        explicit connection(const options_t& options);
        ~connection();

        connection(const connection&) = delete;
        connection& operator=(const connection&) = delete;


        // Observers

        bool is_open() const noexcept;
        ::redisContext* native_handle() noexcept;


        // Execution

        Value exec(std::span<const Value> args);
        Value exec(std::initializer_list<Value> args);


    private:
        ::redisContext* _ctx = nullptr;
    };
}
