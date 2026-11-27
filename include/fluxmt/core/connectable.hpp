#pragma once

// std
#include <concepts>
#include <memory>
#include <string_view>

// ers
#include <erslib/core/type/diagnostic.hpp>

// fluxmt
#include <fluxmt/core/backend.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    template<typename B>
    concept IsConnectable = IsBackend<B> && requires(
        const typename backend_traits<B>::options& options,
        const connection_t<B>& connection,
        std::string_view label
    ) {
        typename backend_traits<B>::default_tx;
        { backend_traits<B>::open(options) } -> std::same_as<std::unique_ptr<connection_t<B>>>;
        { backend_traits<B>::alive(connection) } -> std::convertible_to<bool>;
        { backend_traits<B>::diagnose(label) } -> std::same_as<ers::Diagnostic>;
        { backend_traits<B>::retryable() } -> std::convertible_to<bool>;
    };
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::IsConnectable;
}
