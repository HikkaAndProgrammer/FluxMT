#include "fluxmt/drivers/redis/transaction.hpp"

// std
#include <stdexcept>
#include <string>
#include <utility>

// fluxmt
#include <fluxmt/drivers/redis/error.hpp>


// Implementation --------------------------------------------------------------

// Member functions

flux::impl::redis::transaction::transaction(connection& conn, std::string_view) :
    _conn(conn) {
}

flux::impl::redis::transaction::~transaction() try {
    abort();
} catch (...) {
}


// Before the first queue()

void flux::impl::redis::transaction::watch(std::string_view key) {
    if (_state != EState::Open)
        throw std::logic_error("watch() must come before the first queue()");

    _conn.exec({ "WATCH", std::string(key) });
    _watching = true;
}

flux::Value flux::impl::redis::transaction::exec(std::span<const Value> args) {
    if (_state != EState::Open)
        throw std::logic_error("exec() is only available before the first queue(), a queued answer comes with commit()");

    return _conn.exec(args);
}
flux::Value flux::impl::redis::transaction::exec(std::initializer_list<Value> args) {
    return exec(std::span(args.begin(), args.end()));
}


// Queued commands

void flux::impl::redis::transaction::queue(std::span<const Value> args) {
    if (_state == EState::Done)
        throw std::logic_error("the transaction is finished");

    if (_state == EState::Open) {
        _conn.exec({ "MULTI" });
        _state = EState::Queued;
    }


    auto reply = _conn.exec(args);
    const auto* status = reply.get_if<std::string>();

    if (!status || *status != "QUEUED")
        throw error(EDbError::Other, "unexpected reply to a queued command");
}
void flux::impl::redis::transaction::queue(std::initializer_list<Value> args) {
    queue(std::span(args.begin(), args.end()));
}

const std::vector<flux::Value>& flux::impl::redis::transaction::results() const noexcept {
    return _results;
}


// Modifiers

void flux::impl::redis::transaction::commit() {
    if (_state == EState::Done)
        return;

    if (_state == EState::Open) {
        if (_watching)
            _conn.exec({ "UNWATCH" });

        _state = EState::Done;
        return;
    }


    auto reply = _conn.exec({ "EXEC" });
    _state = EState::Done;


    const auto* items = reply.get_if<value_array_t>();
    if (!items)
        throw error(EDbError::SerializationFailure, "a WATCHed key was modified, EXEC did nothing");

    _results = *items;
}

void flux::impl::redis::transaction::abort() {
    if (_state == EState::Done)
        return;


    auto was = std::exchange(_state, EState::Done);

    if (was == EState::Queued)
        _conn.exec({ "DISCARD" });
    else if (_watching)
        _conn.exec({ "UNWATCH" });
}
