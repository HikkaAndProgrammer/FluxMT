#pragma once

// std
#include <concepts>
#include <initializer_list>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    enum class EFeature : ers::u8 {
        Sql,
        Transactions,
        Returning,
        Upsert,
        DistinctOn,
        NativeArrays,
        LENGTH,
    };

    static_assert(static_cast<ers::u8>(EFeature::LENGTH) < 64, "feature_set holds at most 64 features");


    struct feature_set {
        ers::u64 bits = 0;


        constexpr feature_set() noexcept = default;

        consteval feature_set(std::initializer_list<EFeature> list) {
            for (const auto it : list)
                bits |= static_cast<ers::u64>(1) << static_cast<ers::u8>(it);
        }


        constexpr bool contains(EFeature what) const noexcept {
            return bits & (static_cast<ers::u64>(1) << static_cast<ers::u8>(what));
        }
    };


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


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::EFeature;
    using impl::feature_set;
    using impl::Backend;
    using impl::Supports;
    using impl::connection_t;
}
