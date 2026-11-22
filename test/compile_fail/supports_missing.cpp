// std
#include <string_view>

// fluxmt
#include <fluxmt/core.hpp>


namespace {
    struct no_returning {};

    template<flux::Backend B>
        requires flux::Supports<B, flux::EFeature::Returning>
    void insert_returning() {}
}

template<>
struct flux::impl::backend_traits<no_returning> {
    static constexpr std::string_view name = "no_returning";
    static constexpr feature_set features { EFeature::Sql };

    using connection = int;
};

void use() {
    insert_returning<no_returning>();
}
