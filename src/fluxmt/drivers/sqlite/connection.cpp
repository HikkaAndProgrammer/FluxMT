#include "fluxmt/drivers/sqlite/connection.hpp"

// std
#include <algorithm>
#include <format>
#include <memory>

// sqlite
#include <sqlite3.h>

// fluxmt
#include <fluxmt/drivers/sqlite/error.hpp>
#include <fluxmt/drivers/sqlite/params.hpp>


// Details ---------------------------------------------------------------------

namespace {
    using flux::impl::Value;
    using flux::impl::sqlite::row_t;

    using stmt_ptr = std::unique_ptr<sqlite3_stmt, int(*)(sqlite3_stmt*)>;


    bool is_blank(std::string_view text) {
        return std::ranges::all_of(text, [](char c) {
            return c == ';' || c == ' ' || c == '\t' || c == '\n' || c == '\r';
        });
    }

    row_t read_row(sqlite3_stmt* stmt, int columns) {
        row_t row;
        row.reserve(static_cast<size_t>(columns));

        for (int i = 0; i < columns; ++i) {
            switch (sqlite3_column_type(stmt, i)) {
                case SQLITE_INTEGER:
                    row.emplace_back(static_cast<ers::i64>(sqlite3_column_int64(stmt, i)));
                    break;

                case SQLITE_FLOAT:
                    row.emplace_back(static_cast<ers::f64>(sqlite3_column_double(stmt, i)));
                    break;

                case SQLITE_TEXT: {
                    const auto* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                    row.emplace_back(std::string(text, static_cast<size_t>(sqlite3_column_bytes(stmt, i))));
                    break;
                }

                case SQLITE_BLOB: {
                    const auto* bytes = static_cast<const std::byte*>(sqlite3_column_blob(stmt, i));
                    const auto size = static_cast<size_t>(sqlite3_column_bytes(stmt, i));
                    row.emplace_back(flux::impl::value_bytes_t(bytes, bytes + size));
                    break;
                }

                default:
                    row.emplace_back();
                    break;
            }
        }

        return row;
    }
}


// Implementation --------------------------------------------------------------

// Member functions

flux::impl::sqlite::connection::connection(const options_t& options) {
    int flags = SQLITE_OPEN_URI | SQLITE_OPEN_NOMUTEX;
    flags |= options.read_only
        ? SQLITE_OPEN_READONLY
        : (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE);

    if (int rc = sqlite3_open_v2(options.path.c_str(), &_db, flags, nullptr); rc != SQLITE_OK) {
        auto failure = last_error(_db, rc);
        sqlite3_close_v2(_db);
        _db = nullptr;

        throw failure;
    }

    try {
        sqlite3_busy_timeout(_db, static_cast<int>(options.busy_timeout.count()));
        sqlite3_db_config(_db, SQLITE_DBCONFIG_DQS_DML, 0, nullptr);
        sqlite3_db_config(_db, SQLITE_DBCONFIG_DQS_DDL, 0, nullptr);
        exec_script(options.foreign_keys ? "PRAGMA foreign_keys = ON" : "PRAGMA foreign_keys = OFF");

        if (options.wal && !options.read_only && !options.in_memory())
            exec_script("PRAGMA journal_mode = WAL; PRAGMA synchronous = NORMAL");
    } catch (...) {
        sqlite3_close_v2(_db);
        _db = nullptr;

        throw;
    }
}

flux::impl::sqlite::connection::~connection() {
    if (_db)
        sqlite3_close_v2(_db);
}


// Observers

bool flux::impl::sqlite::connection::is_open() const noexcept {
    return _db != nullptr;
}

bool flux::impl::sqlite::connection::in_transaction() const noexcept {
    return _db && !sqlite3_get_autocommit(_db);
}

sqlite3* flux::impl::sqlite::connection::native_handle() noexcept {
    return _db;
}


// Execution

flux::impl::sqlite::result_t flux::impl::sqlite::connection::exec(std::string_view sql, std::span<const Value> params) {
    sqlite3_stmt* raw = nullptr;
    const char* tail = nullptr;

    if (sqlite3_prepare_v3(_db, sql.data(), static_cast<int>(sql.size()), 0, &raw, &tail) != SQLITE_OK)
        throw last_error(_db);


    stmt_ptr stmt(raw, &sqlite3_finalize);
    result_t out;

    if (!stmt)
        return out;

    if (!is_blank({ tail, static_cast<size_t>(sql.data() + sql.size() - tail) }))
        throw error(SQLITE_MISUSE, "exec() takes one statement, use exec_script() for several");

    if (auto expected = static_cast<size_t>(sqlite3_bind_parameter_count(stmt.get())); expected != params.size()) {
        throw error(SQLITE_RANGE, std::format("statement has {} parameters, {} given",
            expected, params.size()));
    }


    for (size_t i = 0; i < params.size(); ++i)
        bind(stmt.get(), static_cast<int>(i) + 1, params[i]);

    int columns = sqlite3_column_count(stmt.get());
    for (int i = 0; i < columns; ++i)
        out.columns.emplace_back(sqlite3_column_name(stmt.get(), i));


    auto changes_before = sqlite3_total_changes64(_db);

    {
        int rc = sqlite3_step(stmt.get());

        for (; rc == SQLITE_ROW; rc = sqlite3_step(stmt.get()))
            out.rows.push_back(read_row(stmt.get(), columns));

        if (rc != SQLITE_DONE)
            throw last_error(_db, rc);
    }

    if (sqlite3_total_changes64(_db) != changes_before) {
        out.affected = static_cast<ers::i64>(sqlite3_changes64(_db));
        out.last_insert_rowid = static_cast<ers::i64>(sqlite3_last_insert_rowid(_db));
    }


    return out;
}
flux::impl::sqlite::result_t flux::impl::sqlite::connection::exec(std::string_view sql, std::initializer_list<Value> params) {
    return exec(sql, std::span<const Value>(params.begin(), params.end()));
}

void flux::impl::sqlite::connection::exec_script(std::string_view sql) {
    std::string owned(sql);

    if (int rc = sqlite3_exec(_db, owned.c_str(), nullptr, nullptr, nullptr); rc != SQLITE_OK)
        throw last_error(_db, rc);
}
