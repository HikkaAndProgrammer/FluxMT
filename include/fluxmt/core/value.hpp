#pragma once

// std
#include <array>
#include <compare>
#include <concepts>
#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/time.hpp>
#include <erslib/core/type/uuid.hpp>

// export
#include <fluxmt/export.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    class Value;

    using value_bytes_t = std::vector<std::byte>;
    using value_array_t = std::vector<Value>;


    struct custom_value_t {
        std::string type;
        std::string text;

        bool operator==(const custom_value_t&) const = default;
    };


    class FLUXMT_EXPORT Value {
    public:
        using storage_t = std::variant<
            std::monostate,
            bool,
            ers::i64,
            ers::u64,
            ers::f64,
            std::string,
            value_bytes_t,
            ers::date_t,
            ers::timestamp_t,
            ers::uuid,
            value_array_t,
            custom_value_t
        >;


        // Member functions

        Value() = default;

        template<typename T>
            requires (!std::same_as<std::remove_cvref_t<T>, Value> && std::constructible_from<storage_t, T>)
        Value(T&& what) : _storage(std::forward<T>(what)) {
        }


        // Observers

        bool is_null() const noexcept;
        size_t index() const noexcept;

        template<typename T>
        const T* get_if() const noexcept {
            return std::get_if<T>(&_storage);
        }

        template<typename Fn>
        decltype(auto) visit(Fn&& fn) const {
            return std::visit(std::forward<Fn>(fn), _storage);
        }


        // Member operators

        bool operator==(const Value& other) const;


    private:
        storage_t _storage;
    };
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::Value;
    using impl::custom_value_t;
    using impl::value_array_t;
    using impl::value_bytes_t;
}
