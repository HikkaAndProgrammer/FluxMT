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
    template<typename T, typename Backend>
    concept IsNativeTransaction = IsBackend<Backend>
        && std::constructible_from<T, connection_t<Backend>&, std::string_view>
        && requires(T& native) {
            native.commit();
            native.abort();
        };
}


namespace flux::impl {
    template<IsBackend Backend, IsNativeTransaction<Backend> NativeTx>
    class Tx {
    public:
        using backend = Backend;
        using native_type = NativeTx;


        // Member functions

        Tx(connection_t<Backend>& conn, std::string_view label) :
            _native(conn, label) {
        }

        Tx(const Tx&) = delete;
        Tx& operator=(const Tx&) = delete;


        // Accessors

        NativeTx& native() noexcept { return _native; }
        const NativeTx& native() const noexcept { return _native; }

        NativeTx& operator*() noexcept { return _native; }
        NativeTx* operator->() noexcept { return &_native; }


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

    template<typename Backend, typename NativeTx>
    inline constexpr bool is_tx_v<Tx<Backend, NativeTx>> = true;


    template<IsBackend Backend, typename T>
    struct tx_for {
        using type = Tx<Backend, T>;
    };

    template<IsBackend Backend, typename Other, typename NativeTx>
    struct tx_for<Backend, Tx<Other, NativeTx>> {
        static_assert(std::same_as<Backend, Other>, "with_tx: the callable asks for a transaction of another backend");

        using type = Tx<Backend, NativeTx>;
    };


    template<typename Fn>
    using first_arg_t = std::remove_cvref_t<typename ers::fn_traits<std::remove_cvref_t<Fn>>::template arg_type<0>>;

    template<typename Fn>
    concept HasFirstArg = requires {
        typename first_arg_t<Fn>;
    };


    template<IsConnectable Backend, typename Fn>
    struct tx_deduce {
        using tx = Tx<Backend, typename ext::backend_traits<Backend>::default_tx>;

        static constexpr bool wrapped = true;
    };

    template<IsConnectable Backend, HasFirstArg Fn>
    struct tx_deduce<Backend, Fn> {
        using param = first_arg_t<Fn>;
        using tx = typename tx_for<Backend, param>::type;

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
    template<IsConnectable Backend, typename Fn>
    auto with_tx(connection_t<Backend>& conn, Fn&& fn, const tx_options_t& opts = {}) {
        using deduced = tx_deduce<Backend, std::remove_cvref_t<Fn>>;
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
                if (round >= opts.retries || !ext::backend_traits<Backend>::retryable())
                    throw;

                std::this_thread::sleep_for(opts.backoff * (round + 1));
            }
        }
    }


    // Pool level

    template<IsConnectable Backend, typename Fn>
    auto with_tx(Pool<Backend>& pool, Fn&& fn, const tx_options_t& opts = {}) {
        auto lease = pool.acquire_or_throw();
        return with_tx<Backend>(*lease, std::forward<Fn>(fn), opts);
    }

    template<IsConnectable Backend, typename Fn>
    auto with_tx(Pool<Backend>& pool, Fn&& fn, std::string_view label) {
        return with_tx(pool, std::forward<Fn>(fn), tx_options_t {
            .label = label,
        });
    }

    template<IsConnectable Backend, typename Fn>
        requires std::invocable<Fn, connection_t<Backend>&>
    auto with_conn(Pool<Backend>& pool, Fn&& fn) {
        auto lease = pool.acquire_or_throw();
        return std::forward<Fn>(fn)(*lease);
    }
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::IsNativeTransaction;
    using impl::Tx;
    using impl::tx_options_t;
    using impl::with_conn;
    using impl::with_tx;
}
