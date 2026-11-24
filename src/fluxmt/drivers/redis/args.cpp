#include "fluxmt/drivers/redis/args.hpp"

// std
#include <concepts>
#include <format>
#include <stdexcept>
#include <type_traits>

// fluxmt
#include <fluxmt/core/value_text.hpp>


// Implementation --------------------------------------------------------------

std::string flux::impl::redis::to_arg(const Value& value) {
    return value.visit([](const auto& v) -> std::string {
        using T = std::remove_cvref_t<decltype(v)>;

        if constexpr (std::same_as<T, std::monostate>)
            throw std::invalid_argument("Redis has not null argument");
        else if constexpr (std::same_as<T, bool>)
            return v ? "1" : "0";
        else if constexpr (std::same_as<T, ers::i64> || std::same_as<T, ers::u64> || std::same_as<T, ers::f64>)
            return std::format("{}", v);
        else if constexpr (std::same_as<T, std::string>)
            return v;
        else if constexpr (std::same_as<T, value_bytes_t>)
            return { reinterpret_cast<const char*>(v.data()), v.size() };
        else if constexpr (std::same_as<T, ers::date_t> || std::same_as<T, ers::timestamp_t> || std::same_as<T, ers::uuid>)
            return to_text(v);
        else if constexpr (std::same_as<T, value_array_t>)
            throw std::invalid_argument("Redis arguments cannot be arrays");
        else if constexpr (std::same_as<T, custom_value_t>)
            return v.text;

        std::unreachable();
    });
}
