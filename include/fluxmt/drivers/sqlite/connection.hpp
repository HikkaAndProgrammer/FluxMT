#pragma once

// std
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/type/general.hpp>

// fluxmt
#include <fluxmt/core/value.hpp>
#include <fluxmt/drivers/sqlite/options.hpp>

// export
#include <fluxmt/export.hpp>


struct sqlite3;


namespace flux::impl::sqlite {
    using row_t = std::vector<Value>;

    struct result_t {
        std::vector<std::string> columns;
        std::vector<row_t> rows;
        ers::i64 affected = 0;
        ers::i64 last_insert_rowid = 0;
    };
}


namespace flux::impl::sqlite {
    class FLUXMT_EXPORT connection {
    public:
        // Member functions

        explicit connection(const options_t& options);
        ~connection();

        connection(const connection&) = delete;
        connection& operator=(const connection&) = delete;


        // Observers

        bool is_open() const noexcept;
        bool in_transaction() const noexcept;
        ::sqlite3* native_handle() noexcept;


        // Execution

        result_t exec(std::string_view sql, std::span<const Value> params = {});
        result_t exec(std::string_view sql, std::initializer_list<Value> params);


        void exec_script(std::string_view sql);


    private:
        ::sqlite3* _db = nullptr;
    };
}
