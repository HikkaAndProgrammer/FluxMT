#include "fluxmt/drivers/sqlite/transaction.hpp"


// transaction_base ------------------------------------------------------------

// Member functions

flux::impl::sqlite::transaction_base::transaction_base(connection& conn, std::string_view begin_sql) :
    _conn(conn) {
    _conn.exec_script(begin_sql);
    _active = true;
}

flux::impl::sqlite::transaction_base::~transaction_base() {
    if (!_active)
        return;

    try {
        if (_conn.in_transaction())
            _conn.exec_script("ROLLBACK");
    } catch (...) {
    }
}


// Execution

flux::impl::sqlite::result_t flux::impl::sqlite::transaction_base::exec(std::string_view sql, std::span<const Value> params) {
    return _conn.exec(sql, params);
}
flux::impl::sqlite::result_t flux::impl::sqlite::transaction_base::exec(std::string_view sql, std::initializer_list<Value> params) {
    return _conn.exec(sql, params);
}


// Modifiers

void flux::impl::sqlite::transaction_base::commit() {
    _conn.exec_script("COMMIT");
    _active = false;
}

void flux::impl::sqlite::transaction_base::abort() {
    if (_conn.in_transaction())
        _conn.exec_script("ROLLBACK");

    _active = false;
}
