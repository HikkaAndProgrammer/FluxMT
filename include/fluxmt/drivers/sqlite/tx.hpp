#pragma once

// std
#include <string_view>
#include <utility>

// fluxmt
#include <fluxmt/core.hpp>
#include <fluxmt/drivers/sqlite/backend.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::sqlite {
    using Database = Pool<backend>;

    using Tx = impl::Tx<backend, transaction>;
    using WriteTx = impl::Tx<backend, write_transaction>;

    template<typename NativeTx>
    using TxOf = impl::Tx<backend, NativeTx>;


    template<typename Fn>
    auto with_tx(connection& conn, Fn&& fn, const tx_options_t& opts = {}) {
        return with_tx_fn<backend>(conn, std::forward<Fn>(fn), opts);
    }

    template<typename Fn>
    auto with_tx(connection& conn, Fn&& fn, std::string_view label) {
        return with_tx_fn<backend>(conn, std::forward<Fn>(fn), tx_options_t { .label = label });
    }

    template<typename Fn>
    auto try_with_tx(connection& conn, Fn&& fn, const tx_options_t& opts = {}) {
        return try_with_tx_fn<backend>(conn, std::forward<Fn>(fn), opts);
    }

    template<typename Fn>
    auto try_with_tx(connection& conn, Fn&& fn, std::string_view label) {
        return try_with_tx_fn<backend>(conn, std::forward<Fn>(fn), tx_options_t { .label = label });
    }
}
