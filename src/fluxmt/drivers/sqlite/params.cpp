#include "fluxmt/drivers/sqlite/params.hpp"

// std
#include <chrono>
#include <concepts>
#include <format>
#include <limits>
#include <stdexcept>
#include <string_view>
#include <type_traits>

// sqlite
#include <sqlite3.h>

// ers
#include <erslib/core/type/uuid.hpp>

// fluxmt
#include <fluxmt/drivers/sqlite/error.hpp>


// Details ---------------------------------------------------------------------

namespace {
    int bind_text(sqlite3_stmt* stmt, int index, std::string_view text) {
        return sqlite3_bind_text64(stmt, index, text.data(), text.size(), SQLITE_TRANSIENT, SQLITE_UTF8);
    }
}


// Implementation --------------------------------------------------------------

void flux::impl::sqlite::bind(sqlite3_stmt* stmt, int index, const Value& value) {
    int rc = value.visit([&](const auto& v) -> int {
        using T = std::remove_cvref_t<decltype(v)>;

        if constexpr (std::same_as<T, std::monostate>)
            return sqlite3_bind_null(stmt, index);
        else if constexpr (std::same_as<T, bool>)
            return sqlite3_bind_int(stmt, index, v ? 1 : 0);
        else if constexpr (std::same_as<T, ers::i64>)
            return sqlite3_bind_int64(stmt, index, v);
        else if constexpr (std::same_as<T, ers::u64>) {
            if (v > static_cast<ers::u64>(std::numeric_limits<sqlite3_int64>::max()))
                throw std::out_of_range("SQLite INTEGER is signed 64-bit, the value does not fit");

            return sqlite3_bind_int64(stmt, index, static_cast<sqlite3_int64>(v));
        } else if constexpr (std::same_as<T, ers::f64>)
            return sqlite3_bind_double(stmt, index, v);
        else if constexpr (std::same_as<T, std::string>)
            return bind_text(stmt, index, v);
        else if constexpr (std::same_as<T, value_bytes_t>) {
            if (v.empty())
                return sqlite3_bind_zeroblob64(stmt, index, 0);

            return sqlite3_bind_blob64(stmt, index, v.data(), v.size(), SQLITE_TRANSIENT);
        } else if constexpr (std::same_as<T, ers::date_t>)
            return bind_text(stmt, index, std::format("{:%F}", v));
        else if constexpr (std::same_as<T, ers::timestamp_t>)
            return bind_text(stmt, index, std::format("{:%F %T}", std::chrono::floor<std::chrono::microseconds>(v)));
        else if constexpr (std::same_as<T, ers::uuid>)
            return bind_text(stmt, index, ers::uuid_to_string(v));
        else if constexpr (std::same_as<T, value_array_t>)
            throw std::invalid_argument("SQLite has not array type");
        else if constexpr (std::same_as<T, custom_value_t>)
            return bind_text(stmt, index, v.text);

        std::unreachable();
    });

    if (rc != SQLITE_OK)
        throw last_error(sqlite3_db_handle(stmt), rc);
}
