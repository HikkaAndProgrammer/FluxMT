#pragma once

// std
#include <concepts>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// ers
#include <erslib/core/type/general.hpp>

// fluxmt
#include <fluxmt/core/value.hpp>


// Details ---------------------------------------------------------------------

namespace flux::impl {
    template<typename T>
    struct value_traits {};

    template<typename T>
    concept HasValueTraits = requires(const T& what) {
        { value_traits<T>::to_value(what) } -> std::same_as<Value>;
    };

    template<typename T>
    concept OptionalLike = !std::is_pointer_v<T> && requires(const T& what) {
        static_cast<bool>(what);
        *what;
    };
}


// Implementation --------------------------------------------------------------

namespace flux::impl {
    template<typename T>
    Value to_value(T&& what) {
        using U = std::remove_cvref_t<T>;

        if constexpr (HasValueTraits<U>)
            return value_traits<U>::to_value(what);
        else if constexpr (std::same_as<U, Value>)
            return what;
        else if constexpr (std::same_as<U, bool>)
            return Value(what);
        else if constexpr (std::signed_integral<U>)
            return Value(static_cast<ers::i64>(what));
        else if constexpr (std::unsigned_integral<U>)
            return Value(static_cast<ers::u64>(what));
        else if constexpr (std::floating_point<U>)
            return Value(static_cast<ers::f64>(what));
        else if constexpr (std::is_enum_v<U>)
            return Value(static_cast<ers::i64>(std::to_underlying(what)));
        else if constexpr (std::convertible_to<const U&, std::string_view>) {
            std::string_view view(what);
            return Value(std::string(view));
        } else if constexpr (std::same_as<U, value_bytes_t>)
            return Value(what);
        else if constexpr (OptionalLike<U>)
            return what ? to_value(*what) : Value();
        else if constexpr (std::ranges::input_range<U>) {
            value_array_t items;

            for (auto&& it : what)
                items.push_back(to_value(it));

            return Value(std::move(items));
        } else
            return Value(std::forward<T>(what));
    }
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::to_value;
    using impl::value_traits;
}
