#include "support/fake_backend.hpp"


// fake_tx ---------------------------------------------------------------------

test::fake_tx::fake_tx(fake_connection& conn, std::string_view) :
    _conn(conn) {
    _conn.log.push_back("begin");
}

test::fake_tx::~fake_tx() {
    if (!_done)
        _conn.log.push_back("rollback");
}

void test::fake_tx::commit() {
    _done = true;
    _conn.log.push_back("commit");
}

void test::fake_tx::abort() {
    _done = true;
    _conn.log.push_back("abort");
}


// backend_traits --------------------------------------------------------------

std::unique_ptr<test::fake_connection> flux::ext::backend_traits<test::fake_backend>::open(const options& opts) {
    if (opts.fail)
        throw std::runtime_error("down");

    return std::make_unique<connection>();
}

ers::Diagnostic flux::ext::backend_traits<test::fake_backend>::diagnose(std::string_view label) try {
    throw;
} catch (const test::fake_retryable& e) {
    return ers::make_error(EDbError::Deadlock, "'{}': {}", label, e.what());
} catch (const std::exception& e) {
    return ers::make_error(EDbError::Other, "'{}': {}", label, e.what());
}

bool flux::ext::backend_traits<test::fake_backend>::retryable() noexcept try {
    throw;
} catch (const test::fake_retryable&) {
    return true;
} catch (...) {
    return false;
}
