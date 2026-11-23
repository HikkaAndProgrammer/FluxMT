#pragma once

// std
#include <string_view>
#include <utility>

// pqxx
#include <pqxx/transaction>

// fluxmt
#include <fluxmt/core.hpp>
#include <fluxmt/psql/backend.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::psql {
    using Database = Pool<backend>;

    using Tx = impl::Tx<backend, pqxx::work>;
    using ReadTx = impl::Tx<backend, pqxx::read_transaction>;

    template<typename N>
    using TxOf = impl::Tx<backend, N>;


    template<typename Fn>
    auto with_tx(pqxx::connection& conn, Fn&& fn, const tx_options_t& opts = {}) {
        return with_tx_fn<backend>(conn, std::forward<Fn>(fn), opts);
    }

    template<typename Fn>
    auto with_tx(pqxx::connection& conn, Fn&& fn, std::string_view label) {
        return with_tx_fn<backend>(conn, std::forward<Fn>(fn), tx_options_t { .label = label });
    }

    template<typename Fn>
    auto try_with_tx(pqxx::connection& conn, Fn&& fn, const tx_options_t& opts = {}) {
        return try_with_tx_fn<backend>(conn, std::forward<Fn>(fn), opts);
    }

    template<typename Fn>
    auto try_with_tx(pqxx::connection& conn, Fn&& fn, std::string_view label) {
        return try_with_tx_fn<backend>(conn, std::forward<Fn>(fn), tx_options_t { .label = label });
    }
}
