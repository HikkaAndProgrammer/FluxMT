#pragma once

// std
#include <concepts>
#include <initializer_list>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>


// Implementation --------------------------------------------------------------

namespace flux {
    enum class EFeature : ers::u8 {
        Sql,
        Transactions,
        Returning,
        Upsert,
        DistinctOn,
        NativeArrays,
    };

    struct feature_set {
        ers::u64 bits = 0;


        consteval feature_set(std::initializer_list<EFeature> list) {
            for (const auto it : list)
                bits |= ers::u64(1) << static_cast<ers::u8>(it);
        }


        constexpr bool contains(EFeature what) const noexcept {
            return bits & (ers::u64(1) << static_cast<ers::u8>(what));
        }
    };


    // Specialized once per driver: its connection, features and dialect.
    template<typename B>
    struct backend_traits;

    template<typename B>
    concept Backend = requires {
        typename backend_traits<B>::connection;
        { backend_traits<B>::name } -> std::convertible_to<std::string_view>;
        { backend_traits<B>::features } -> std::convertible_to<feature_set>;
    };

    template<typename B, EFeature F>
    concept Supports = Backend<B> && backend_traits<B>::features.contains(F);

    template<Backend B>
    using connection_t = typename backend_traits<B>::connection;
}
