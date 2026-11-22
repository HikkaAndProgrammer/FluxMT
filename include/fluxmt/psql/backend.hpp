#pragma once

// std
#include <string_view>

// pqxx
#include <pqxx/connection>

// fluxmt
#include <fluxmt/core/backend.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::psql {
    struct backend {};
}


// Specializations -------------------------------------------------------------

template<>
struct flux::impl::backend_traits<flux::impl::psql::backend> {
    using connection = pqxx::connection;


    static constexpr std::string_view name = "psql";

    static constexpr feature_set features = {
        EFeature::Sql,
        EFeature::Transactions,
        EFeature::Returning,
        EFeature::Upsert,
        EFeature::DistinctOn,
        EFeature::NativeArrays,
    };
};
