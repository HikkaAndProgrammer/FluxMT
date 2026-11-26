#pragma once

// std
#include <concepts>

// fluxmt
#include <fluxmt/core/backend.hpp>
#include <fluxmt/sql/dialect.hpp>


namespace flux::impl {
    template<typename B>
    concept SqlBackend = Supports<B, EFeature::Sql> && requires {
        { backend_traits<B>::dialect() } -> std::convertible_to<const SqlDialect&>;
    };
}
