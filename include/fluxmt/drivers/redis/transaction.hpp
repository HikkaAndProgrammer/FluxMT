#pragma once

// std
#include <initializer_list>
#include <span>
#include <string_view>
#include <vector>

// fluxmt
#include <fluxmt/core/value.hpp>
#include <fluxmt/drivers/redis/connection.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::redis {
    class FLUXMT_EXPORT transaction {
    public:
        // Member functions

        transaction(connection& conn, std::string_view);
        ~transaction();

        transaction(const transaction&) = delete;
        transaction& operator=(const transaction&) = delete;


        // Before the first queue()

        void watch(std::string_view key);

        Value exec(std::span<const Value> args);
        Value exec(std::initializer_list<Value> args);


        // Queued commands: their answers arrive with commit()

        void queue(std::span<const Value> args);
        void queue(std::initializer_list<Value> args);

        const std::vector<Value>& results() const noexcept;


        // Modifiers

        void commit();
        void abort();


    private:
        enum class EState {
            Open,
            Queued,
            Done,
        };


        connection& _conn;
        EState _state = EState::Open;
        bool _watching = false;
        std::vector<Value> _results;
    };
}
