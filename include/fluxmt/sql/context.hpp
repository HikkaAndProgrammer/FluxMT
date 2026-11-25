#pragma once

// std
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core/value.hpp>
#include <fluxmt/sql/dialect.hpp>
#include <fluxmt/sql/statement.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class FLUXMT_EXPORT Context {
    public:
        // Member functions

        explicit Context(const Dialect& dialect) noexcept;
        Context(const Dialect&&) = delete;


        // Observers

        const Dialect& dialect() const noexcept;
        const std::vector<Value>& params() const noexcept;


        // Modifiers

        std::string bind(Value value);

        std::string bind_once(const void* key, Value value);

        // name[.name...]: a part stays bare when it is plain and not reserved, otherwise it is quoted.
        ers::Status append_identifier(std::string_view name);

        Statement finish() &&;


        std::string query;


    private:
        const Dialect* _dialect;
        std::vector<Value> _params;
        std::unordered_map<const void*, std::string> _bound;
    };
}
