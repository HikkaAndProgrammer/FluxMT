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
#include <fluxmt/query/dialect.hpp>
#include <fluxmt/query/statement.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class FLUXMT_EXPORT Context {
    public:
        // Member functions

        explicit Context(const Dialect& dialect) noexcept;
        Context(const Dialect&&) = delete;

        virtual ~Context() = default;


        // Observers

        virtual const Dialect& dialect() const noexcept;
        const std::vector<Value>& params() const noexcept;


        // Modifiers

        std::string bind(Value value);

        std::string bind_once(const void* key, Value value);

        statement_t finish() &&;


        std::string text;


    private:
        const Dialect* _dialect;
        std::vector<Value> _params;
        std::unordered_map<const void*, std::string> _bound;
    };
}
