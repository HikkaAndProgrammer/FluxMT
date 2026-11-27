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
    template<IsConnectable Backend, std::invocable Fn>
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
            return ext::backend_traits<Backend>::diagnose(label.empty() ? "database call"sv : label);
        }
    }


    template<IsConnectable Backend, typename Fn>
    auto try_with_tx(connection_t<Backend>& conn, Fn&& fn, const tx_options_t& opts = {}) {
        return try_invoke<Backend>([&] {
            return with_tx<Backend>(conn, std::forward<Fn>(fn), opts);
        }, opts.label);
    }

    template<IsConnectable Backend, typename Fn>
    auto try_with_tx(Pool<Backend>& pool, Fn&& fn, const tx_options_t& opts = {}) {
        using R = ers::as_result_t<decltype(with_tx<Backend>(
            std::declval<connection_t<Backend>&>(),
            std::declval<Fn>(),
            opts
        ))>;

        auto lease = pool.acquire();
        if (!lease)
            return R(std::move(lease).extract_error());

        return try_with_tx<Backend>(**lease, std::forward<Fn>(fn), opts);
    }

    template<IsConnectable Backend, typename Fn>
    auto try_with_tx(Pool<Backend>& pool, Fn&& fn, std::string_view label) {
        return try_with_tx(pool, std::forward<Fn>(fn), tx_options_t {
            .label = label,
        });
    }

    template<IsConnectable Backend, typename Fn>
        requires std::invocable<Fn, connection_t<Backend>&>
    auto try_with_conn(Pool<Backend>& pool, Fn&& fn) {
        using R = ers::as_result_t<std::invoke_result_t<Fn, connection_t<Backend>&>>;

        auto lease = pool.acquire();
        if (!lease)
            return R(std::move(lease).extract_error());

        return try_invoke<Backend>([&] { return std::forward<Fn>(fn)(**lease); });
    }
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::try_with_conn;
    using impl::try_with_tx;
}
