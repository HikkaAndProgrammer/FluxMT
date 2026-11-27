#pragma once

// std
#include <concepts>
#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>

// ers
#include <erslib/core/trait/result.hpp>
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core/tx.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    template<IsConnectable B, std::invocable Fn>
    auto try_invoke(Fn&& fn, std::string_view label = {}) -> ers::as_result_t<std::invoke_result_t<Fn>> {
        using std::string_view_literals::operator ""sv;
        using R = std::invoke_result_t<Fn>;

        try {
            if constexpr (std::is_void_v<R>) {
                std::invoke(std::forward<Fn>(fn));
                return ers::ok;
            } else {
                return std::invoke(std::forward<Fn>(fn));
            }
        } catch (...) {
            return backend_traits<B>::diagnose(label.empty() ? "database call"sv : label);
        }
    }


    template<IsConnectable B, typename Fn>
    auto try_with_tx_fn(connection_t<B>& conn, Fn&& fn, const tx_options_t& opts = {}) {
        return try_invoke<B>([&] {
            return with_tx_fn<B>(conn, std::forward<Fn>(fn), opts);
        }, opts.label);
    }

    template<IsConnectable B, typename Fn>
    auto try_with_tx(Pool<B>& pool, Fn&& fn, const tx_options_t& opts = {}) {
        using R = ers::as_result_t<decltype(with_tx_fn<B>(
            std::declval<connection_t<B>&>(),
            std::declval<Fn>(),
            opts
        ))>;

        auto lease = pool.acquire();
        if (!lease)
            return R(std::move(lease).extract_error());

        return try_with_tx_fn<B>(**lease, std::forward<Fn>(fn), opts);
    }

    template<IsConnectable B, typename Fn>
    auto try_with_tx(Pool<B>& pool, Fn&& fn, std::string_view label) {
        return try_with_tx(pool, std::forward<Fn>(fn), tx_options_t {
            .label = label,
        });
    }

    template<IsConnectable B, typename Fn>
        requires std::invocable<Fn, connection_t<B>&>
    auto try_with_conn(Pool<B>& pool, Fn&& fn) {
        using R = ers::as_result_t<std::invoke_result_t<Fn, connection_t<B>&>>;

        auto lease = pool.acquire();
        if (!lease)
            return R(std::move(lease).extract_error());

        return try_invoke<B>([&] { return std::forward<Fn>(fn)(**lease); });
    }
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::try_with_conn;
    using impl::try_with_tx;
    using impl::try_with_tx_fn;
}
