#pragma once

// std
#include <chrono>
#include <concepts>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>

// ers
#include <erslib/core/trait/fn.hpp>
#include <erslib/core/trait/result.hpp>
#include <erslib/core/type/time.hpp>

// fluxmt
#include <fluxmt/core/connectable.hpp>
#include <fluxmt/core/pool.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    template<typename N, typename B>
    concept NativeTransaction = Backend<B>
        && std::constructible_from<N, connection_t<B>&, std::string_view>
        && requires(N& native) {
            native.commit();
            native.abort();
        };
}


namespace flux::impl {
    template<Backend B, NativeTransaction<B> N>
    class Tx {
    public:
        using backend = B;
        using native_type = N;


        // Member functions

        Tx(connection_t<B>& conn, std::string_view label) :
            _native(conn, label) {
        }

        Tx(const Tx&) = delete;
        Tx& operator=(const Tx&) = delete;


        // Accessors

        N& native() noexcept { return _native; }
        const N& native() const noexcept { return _native; }

        N& operator*() noexcept { return _native; }
        N* operator->() noexcept { return &_native; }


        // Modifiers

        void commit() { _native.commit(); }
        void abort() { _native.abort(); }


    private:
        native_type _native;
    };
}


// Transaction traits

namespace flux::impl {
    template<typename T>
    inline constexpr bool is_tx_v = false;

    template<typename B, typename N>
    inline constexpr bool is_tx_v<Tx<B, N>> = true;


    template<typename B, typename P>
    struct tx_for {
        using type = Tx<B, P>;
    };

    template<typename B, typename Other, typename N>
    struct tx_for<B, Tx<Other, N>> {
        static_assert(std::same_as<B, Other>, "with_tx: the callable asks for a transaction of another backend");

        using type = Tx<B, N>;
    };


    template<typename Fn>
    using first_arg_t = std::remove_cvref_t<typename ers::fn_traits<std::remove_cvref_t<Fn>>::template arg_type<0>>;

    template<typename Fn>
    concept HasFirstArg = requires {
        typename first_arg_t<Fn>;
    };


    template<typename B, typename Fn>
    struct tx_deduce {
        using tx = Tx<B, typename backend_traits<B>::default_tx>;

        static constexpr bool wrapped = true;
    };

    template<typename B, HasFirstArg Fn>
    struct tx_deduce<B, Fn> {
        using param = first_arg_t<Fn>;
        using tx = typename tx_for<B, param>::type;

        static constexpr bool wrapped = is_tx_v<param>;
    };


    struct tx_options_t {
        std::string_view label;
        ers::u8 retries = 0;
        ers::ms_t backoff = std::chrono::milliseconds(20);
    };


    template<bool Wrapped, typename Fn, typename T>
    decltype(auto) call_with_tx(Fn&& fn, T& tx) {
        if constexpr (Wrapped)
            return std::forward<Fn>(fn)(tx);
        else
            return std::forward<Fn>(fn)(tx.native());
    }
}


namespace flux::impl {
    template<Connectable B, typename Fn>
    auto with_tx_fn(connection_t<B>& conn, Fn&& fn, const tx_options_t& opts = {}) {
        using deduced = tx_deduce<B, std::remove_cvref_t<Fn>>;
        using tx_type = typename deduced::tx;

        using return_type = decltype(call_with_tx<deduced::wrapped>(
            std::declval<std::remove_reference_t<Fn>&>(),
            std::declval<tx_type&>()
        ));

        auto attempt = [&]() -> return_type {
            tx_type tx(conn, opts.label);

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
                if (round >= opts.retries || !backend_traits<B>::retryable())
                    throw;

                std::this_thread::sleep_for(opts.backoff * (round + 1));
            }
        }
    }


    // Pool level

    template<Connectable B, typename Fn>
    auto with_tx(Pool<B>& pool, Fn&& fn, const tx_options_t& opts = {}) {
        auto lease = pool.acquire_or_throw();
        return with_tx_fn<B>(*lease, std::forward<Fn>(fn), opts);
    }

    template<Connectable B, typename Fn>
    auto with_tx(Pool<B>& pool, Fn&& fn, std::string_view label) {
        return with_tx(pool, std::forward<Fn>(fn), tx_options_t {
            .label = label,
        });
    }

    template<Connectable B, typename Fn>
        requires std::invocable<Fn, connection_t<B>&>
    auto with_conn(Pool<B>& pool, Fn&& fn) {
        auto lease = pool.acquire_or_throw();
        return std::forward<Fn>(fn)(*lease);
    }
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::NativeTransaction;
    using impl::Tx;
    using impl::tx_options_t;
    using impl::with_conn;
    using impl::with_tx;
    using impl::with_tx_fn;
}
