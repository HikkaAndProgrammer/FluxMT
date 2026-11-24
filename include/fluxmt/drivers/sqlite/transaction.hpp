#pragma once

// std
#include <initializer_list>
#include <span>
#include <string_view>

// fluxmt
#include <fluxmt/drivers/sqlite/connection.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::sqlite {
    enum class EBegin {
        Deferred,
        Immediate,
    };


    class FLUXMT_EXPORT transaction_base {
    public:
        // Member functions

        transaction_base(const transaction_base&) = delete;
        transaction_base& operator=(const transaction_base&) = delete;


        // Execution

        result_t exec(std::string_view sql, std::span<const Value> params = {});
        result_t exec(std::string_view sql, std::initializer_list<Value> params);


        // Modifiers

        void commit();
        void abort();


    protected:
        transaction_base(connection& conn, std::string_view begin_sql);
        ~transaction_base();


    private:
        connection& _conn;
        bool _active = false;
    };
}


namespace flux::impl::sqlite {
    class FLUXMT_EXPORT transaction final : public transaction_base {
    public:
        transaction(connection& conn, std::string_view) :
            transaction_base(conn, "BEGIN") {
        }
    };


    class FLUXMT_EXPORT write_transaction final : public transaction_base {
    public:
        write_transaction(connection& conn, std::string_view) :
            transaction_base(conn, "BEGIN IMMEDIATE") {
        }
    };
}
