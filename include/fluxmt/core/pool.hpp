#pragma once

// std
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/core/type/time.hpp>

// fluxmt
#include <fluxmt/core/connectable.hpp>
#include <fluxmt/core/error.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    struct pool_options_t {
        size_t min_size = 1;
        size_t max_size = 4;
        ers::ms_t acquire_timeout = std::chrono::seconds(5);
        ers::ms_t idle_timeout = std::chrono::minutes(5);
    };

    template<typename Traits>
    struct entry_t {
        using connection_type = typename Traits::connection;


        connection_type* conn;
        ers::timestamp_t last_used;
    };

    template<typename Traits>
    struct state_t {
        using connection_type = typename Traits::connection;
        using connection_ptr = std::unique_ptr<typename Traits::connection>;
        using options_type = typename Traits::options;
        using entry_type = entry_t<Traits>;


        options_type opts;
        pool_options_t pool_opts;

        mutable std::mutex mutex;
        std::condition_variable cv;
        std::vector<connection_ptr> all;
        std::deque<entry_type> idle;


        // Member functions

        state_t(options_type opts, pool_options_t pool_opts) :
            opts(std::move(opts)),
            pool_opts(pool_opts) {
        }


        // Observers

        bool is_stale(const entry_type& entry, ers::timestamp_t now) const noexcept {
            return now - entry.last_used > pool_opts.idle_timeout;
        }

        bool is_expired(const entry_type& entry, ers::timestamp_t now) const noexcept {
            return is_stale(entry, now) && all.size() > pool_opts.min_size;
        }


        // Modifiers

        ers::Result<connection_type*> spawn() try {
            auto conn = Traits::open(opts);
            auto raw = conn.get();

            all.push_back(std::move(conn));
            return raw;
        } catch (...) {
            return Traits::diagnose("open connection");
        }

        void discard(connection_type* conn) noexcept {
            std::erase_if(all, [&](const connection_ptr& owned) {
                return owned.get() == conn;
            });
        }


        void init() {
            std::scoped_lock lock(mutex);

            for (size_t i = 0; i < pool_opts.min_size; ++i) {
                auto conn = spawn();
                if (!conn)
                    break;

                idle.push_back({ *conn, ers::clock_t::now() });
            }
        }


        ers::Result<connection_type*> acquire() {
            std::unique_lock lock(mutex);


            auto can_supply = [this] {
                return !idle.empty() || all.size() < pool_opts.max_size;
            };

            if (!cv.wait_for(lock, pool_opts.acquire_timeout, can_supply))
                return ers::make_error(EDbError::PoolTimeout, "Timed out acquiring a pooled connection");


            auto now = ers::clock_t::now();

            while (!idle.empty()) {
                auto entry = idle.front();
                idle.pop_front();

                if (!is_expired(entry, now) && Traits::alive(*entry.conn))
                    return entry.conn;

                discard(entry.conn);
            }


            return spawn();
        }

        void release(connection_type* conn) noexcept {
            {
                std::scoped_lock lock(mutex);
                idle.push_back({ conn, ers::clock_t::now() });
            }

            cv.notify_one();
        }

        void maintain() {
            std::scoped_lock lock(mutex);

            std::deque<entry_t<Traits>> kept;
            auto now = ers::clock_t::now();

            for (const auto& entry : idle) {
                if (is_expired(entry, now))
                    discard(entry.conn);
                else
                    kept.push_back(entry);
            }

            idle = std::move(kept);
        }
    };


    template<typename Traits>
    class Lease {
        template<IsConnectable Backend>
        friend class Pool;


    public:
        using connection_type = typename Traits::connection;
        using state_type = state_t<Traits>;


        // Member functions

        Lease(Lease&& other) noexcept :
            _state(std::move(other._state)),
            _conn(std::exchange(other._conn, nullptr)) {
        }
        Lease& operator=(Lease&& other) noexcept {
            if (this != &other) {
                _release();
                _state = std::move(other._state);
                _conn = std::exchange(other._conn, nullptr);
            }

            return *this;
        }

        Lease(const Lease&) = delete;
        Lease& operator=(const Lease&) = delete;

        ~Lease() {
            _release();
        }


        // Accessors

        connection_type& operator*() noexcept { return *_conn; }
        connection_type* operator->() noexcept { return _conn; }


    private:
        std::shared_ptr<state_type> _state;
        connection_type* _conn;


        Lease(std::shared_ptr<state_type> state, connection_type* conn) noexcept :
            _state(std::move(state)),
            _conn(conn) {
        }

        void _release() noexcept {
            if (!_state)
                return;

            auto state = std::exchange(_state, nullptr);
            state->release(std::exchange(_conn, nullptr));
        }
    };
}


namespace flux::impl {
    template<IsConnectable Backend>
    class Pool {
        using traits = backend_traits<Backend>;


    public:
        using connection_type = typename traits::connection;
        using connection_ptr = std::unique_ptr<typename traits::connection>;
        using options_type = typename traits::options;
        using state_type = state_t<traits>;
        using lease_type = Lease<traits>;


        // Member functions

        explicit Pool(options_type opts, pool_options_t pool_opts = {}) :
            _state(std::make_shared<state_type>(std::move(opts), pool_opts)) {
            _state->init();
        }


        // Connections

        ers::Result<lease_type> acquire() {
            auto conn = ERS_TRY(_state->acquire());
            return lease_type(_state, conn);
        }

        lease_type acquire_or_throw() {
            return acquire().or_throw();
        }


        // Modifiers

        void maintain() {
            _state->maintain();
        }


    private:
        std::shared_ptr<state_type> _state;
    };
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::pool_options_t;
    using impl::Pool;
}
