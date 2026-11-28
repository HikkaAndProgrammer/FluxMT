#pragma once

// std
#include <concepts>
#include <initializer_list>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>


// Extension points ------------------------------------------------------------

namespace flux::ext {
    template<typename Backend>
    struct backend_traits;
}


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


    template<typename T>
    concept IsBackend = requires {
        typename ext::backend_traits<T>::connection;
        { ext::backend_traits<T>::name } -> std::convertible_to<std::string_view>;
        { ext::backend_traits<T>::features } -> std::convertible_to<feature_set>;
    };

    template<typename Backend, EFeature Feature>
    concept SupportsFeature = IsBackend<Backend> && ext::backend_traits<Backend>::features.contains(Feature);

    template<IsBackend Backend>
    using connection_t = typename ext::backend_traits<Backend>::connection;
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::EFeature;
    using impl::feature_set;
    using impl::IsBackend;
    using impl::SupportsFeature;
    using impl::connection_t;
}
