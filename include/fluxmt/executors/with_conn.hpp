#pragma once

// std
#include <concepts>
#include <type_traits>
#include <utility>

// ers
#include <erslib/core/trait/result.hpp>
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core/connectable.hpp>
#include <fluxmt/core/pool.hpp>
#include <fluxmt/core/try_invoke.hpp>


namespace flux::impl {
    struct with_conn_fn {
        template<IsConnectable Backend, typename Fn>
            requires std::invocable<Fn, connection_t<Backend>&>
        auto operator()(Pool<Backend>& pool, Fn&& fn) const {
            auto lease = pool.acquire_or_throw();
            return std::forward<Fn>(fn)(*lease);
        }
    };

    inline constexpr with_conn_fn with_conn {};
}


namespace flux::impl {
    struct try_with_conn_fn {
        template<IsConnectable Backend, typename Fn>
            requires std::invocable<Fn, connection_t<Backend>&>
        auto operator()(Pool<Backend>& pool, Fn&& fn) const {
            using R = ers::as_result_t<std::invoke_result_t<Fn, connection_t<Backend>&>>;

            auto lease = pool.acquire();
            if (!lease)
                return R(std::move(lease).extract_error());

            return try_invoke<Backend>([&] {
                return std::forward<Fn>(fn)(**lease);
            });
        }
    };

    inline constexpr try_with_conn_fn try_with_conn {};
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::try_with_conn;
    using impl::with_conn;
}
