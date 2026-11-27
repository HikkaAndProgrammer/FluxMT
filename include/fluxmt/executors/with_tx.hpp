#pragma once

// std
#include <concepts>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

// ers
#include <erslib/core/trait/result.hpp>
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core/connectable.hpp>
#include <fluxmt/core/pool.hpp>
#include <fluxmt/core/try_invoke.hpp>
#include <fluxmt/core/tx.hpp>


namespace flux::impl {
    struct with_tx_fn {
        template<IsConnectable Backend, typename Fn>
        auto operator()(Lease<ext::backend_traits<Backend>>& lease, Fn&& fn, const tx_options_t& opts = {}) const {
            using deduced = tx_deduce<Backend, std::remove_cvref_t<Fn>>;
            using tx_type = typename deduced::tx;

            using return_type = decltype(call_with_tx<deduced::wrapped>(
                std::declval<std::remove_reference_t<Fn>&>(),
                std::declval<tx_type&>()
            ));

            auto attempt = [&]() -> return_type {
                tx_type tx(*lease, opts.label);

                if constexpr (std::is_void_v<return_type>) {
                    call_with_tx<deduced::wrapped>(fn, tx);
                    tx.commit();
                } else {
                    auto result = call_with_tx<deduced::wrapped>(fn, tx);

                    if constexpr (ers::is_result_v<std::remove_cvref_t<return_type>>) {
                        if (!result) {
                            tx.abort();
                            return result;
                        }
                    }

                    tx.commit();
                    return result;
                }
            };

            for (ers::u8 round = 0;; ++round) {
                try {
                    return attempt();
                } catch (...) {
                    if (round >= opts.retries || !ext::backend_traits<Backend>::retryable())
                        throw;

                    std::this_thread::sleep_for(opts.backoff * (round + 1));
                }
            }
        }

        template<IsConnectable Backend, typename Fn>
        auto operator()(Pool<Backend>& pool, Fn&& fn, const tx_options_t& opts = {}) const {
            auto lease = pool.acquire_or_throw();
            return (*this)(lease, std::forward<Fn>(fn), opts);
        }

        template<IsConnectable Backend, typename Fn>
        auto operator()(Pool<Backend>& pool, Fn&& fn, std::string_view label) const {
            return (*this)(pool, std::forward<Fn>(fn), tx_options_t {
                .label = label,
            });
        }
    };

    inline constexpr with_tx_fn with_tx {};
}


namespace flux::impl {
    struct try_with_tx_fn {
        template<IsConnectable Backend, typename Fn>
        auto operator()(Lease<ext::backend_traits<Backend>>& lease, Fn&& fn, const tx_options_t& opts = {}) const {
            return try_invoke<Backend>([&] {
                return with_tx(lease, std::forward<Fn>(fn), opts);
            }, opts.label);
        }

        template<IsConnectable Backend, typename Fn>
        auto operator()(Pool<Backend>& pool, Fn&& fn, const tx_options_t& opts = {}) const {
            using R = ers::as_result_t<decltype(with_tx(
                std::declval<typename Pool<Backend>::lease_type&>(),
                std::declval<Fn>(),
                opts
            ))>;

            auto lease = pool.acquire();
            if (!lease)
                return R(std::move(lease).extract_error());

            return (*this)(*lease, std::forward<Fn>(fn), opts);
        }

        template<IsConnectable Backend, typename Fn>
        auto operator()(Pool<Backend>& pool, Fn&& fn, std::string_view label) const {
            return (*this)(pool, std::forward<Fn>(fn), tx_options_t {
                .label = label,
            });
        }
    };

    inline constexpr try_with_tx_fn try_with_tx {};
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::try_with_tx;
    using impl::with_tx;
}
