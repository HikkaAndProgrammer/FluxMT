#pragma once

// std
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/type/diagnostic.hpp>

// fluxmt
#include <fluxmt/core.hpp>


namespace test {
    struct fake_backend {};

    struct fake_connection {
        bool open = true;
        std::vector<std::string> log;
    };

    struct fake_options {
        bool fail = false;
    };

    struct fake_retryable : std::runtime_error {
        using runtime_error::runtime_error;
    };


    class fake_tx {
    public:
        fake_tx(fake_connection& conn, std::string_view);
        ~fake_tx();

        fake_tx(const fake_tx&) = delete;
        fake_tx& operator=(const fake_tx&) = delete;

        void commit();
        void abort();


    private:
        fake_connection& _conn;
        bool _done = false;
    };
}


template<>
struct flux::ext::backend_traits<test::fake_backend> {
    using connection = test::fake_connection;
    using options = test::fake_options;
    using default_tx = test::fake_tx;

    static constexpr std::string_view name = "fake";
    static constexpr feature_set features { EFeature::Sql, EFeature::Transactions };


    static std::unique_ptr<connection> open(const options& opts);
    static bool alive(const connection& conn) noexcept { return conn.open; }
    static ers::Diagnostic diagnose(std::string_view label);
    static bool retryable() noexcept;
};
