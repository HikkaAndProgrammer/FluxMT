#include "fluxmt/drivers/redis/connection.hpp"

// std
#include <format>
#include <memory>
#include <string>
#include <vector>

// hiredis
#include <hiredis/hiredis.h>

// fluxmt
#include <fluxmt/drivers/redis/args.hpp>
#include <fluxmt/drivers/redis/error.hpp>


// Details ---------------------------------------------------------------------

namespace {
    using flux::EDbError;
    using flux::Value;
    using flux::impl::redis::error;


    struct reply_deleter {
        void operator()(redisReply* reply) const noexcept {
            freeReplyObject(reply);
        }
    };

    using reply_ptr = std::unique_ptr<redisReply, reply_deleter>;


    timeval to_timeval(ers::ms_t value) {
        timeval out {};

        out.tv_sec = static_cast<time_t>(value.count() / 1000);
        out.tv_usec = static_cast<suseconds_t>((value.count() % 1000) * 1000);

        return out;
    }

    error transport_error(const redisContext* ctx) {
        return { flux::impl::redis::classify_transport(ctx->err), ctx->errstr };
    }

    Value to_value(const redisReply& reply) {
        switch (reply.type) {
            case REDIS_REPLY_NIL:
                return {};

            case REDIS_REPLY_INTEGER:
                return static_cast<ers::i64>(reply.integer);

            case REDIS_REPLY_STATUS:
            case REDIS_REPLY_STRING:
                return std::string(reply.str, reply.len);

            case REDIS_REPLY_ARRAY: {
                flux::value_array_t items;
                items.reserve(reply.elements);

                for (size_t i = 0; i < reply.elements; ++i)
                    items.push_back(to_value(*reply.element[i]));

                return Value(std::move(items));
            }

            case REDIS_REPLY_ERROR: {
                std::string text(reply.str, reply.len);
                throw error(flux::impl::redis::classify_reply(text), text);
            }

            default:
                throw error(EDbError::Other, std::format("unsupported reply type {}", reply.type));
        }
    }
}


// Implementation --------------------------------------------------------------

// Member functions

flux::impl::redis::connection::connection(const options_t& options) {
    redisOptions raw {};

    if (options.unix_socket.empty()) {
        REDIS_OPTIONS_SET_TCP(&raw, options.host.c_str(), options.port);
    } else {
        REDIS_OPTIONS_SET_UNIX(&raw, options.unix_socket.c_str());
    }


    auto connect_timeout = to_timeval(options.connect_timeout);
    auto command_timeout = to_timeval(options.command_timeout);

    raw.connect_timeout = &connect_timeout;
    if (options.command_timeout.count() > 0)
        raw.command_timeout = &command_timeout;


    _ctx = redisConnectWithOptions(&raw);
    if (!_ctx)
        throw error(EDbError::Other, "cannot allocate a Redis context");

    if (_ctx->err) {
        auto failure = transport_error(_ctx);
        redisFree(_ctx);
        _ctx = nullptr;

        throw failure;
    }


    try {
        if (!options.password.empty()) {
            if (options.username.empty())
                exec({ "AUTH", options.password });
            else
                exec({ "AUTH", options.username, options.password });
        }

        if (options.db != 0)
            exec({ "SELECT", options.db });

        if (!options.client_name.empty())
            exec({ "CLIENT", "SETNAME", options.client_name });
    } catch (...) {
        redisFree(_ctx);
        _ctx = nullptr;

        throw;
    }
}

flux::impl::redis::connection::~connection() {
    if (_ctx)
        redisFree(_ctx);
}


// Observers

bool flux::impl::redis::connection::is_open() const noexcept {
    return _ctx && _ctx->err == 0;
}

redisContext* flux::impl::redis::connection::native_handle() noexcept {
    return _ctx;
}


// Execution

Value flux::impl::redis::connection::exec(std::span<const Value> args) {
    if (args.empty())
        throw error(EDbError::Other, "empty command");

    if (!is_open())
        throw error(EDbError::ConnectionLost, "connection is closed");


    std::vector<std::string> storage;
    storage.reserve(args.size());

    for (const auto& arg : args)
        storage.push_back(to_arg(arg));


    std::vector<const char*> argv;
    std::vector<size_t> lengths;

    for (const auto& item : storage) {
        argv.push_back(item.data());
        lengths.push_back(item.size());
    }


    reply_ptr reply(static_cast<redisReply*>(
        redisCommandArgv(_ctx, static_cast<int>(argv.size()), argv.data(), lengths.data())
    ));

    if (!reply)
        throw transport_error(_ctx);


    return to_value(*reply);
}

Value flux::impl::redis::connection::exec(std::initializer_list<Value> args) {
    return exec(std::span<const Value>(args.begin(), args.end()));
}
