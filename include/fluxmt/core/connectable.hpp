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
    template<IsBackend Backend>
    concept IsConnectable = requires(
        const typename ext::backend_traits<Backend>::options& options,
        const connection_t<Backend>& connection,
        std::string_view label
    ) {
        typename ext::backend_traits<Backend>::default_tx;
        { ext::backend_traits<Backend>::open(options) } -> std::same_as<std::unique_ptr<connection_t<Backend>>>;
        { ext::backend_traits<Backend>::alive(connection) } -> std::convertible_to<bool>;
        { ext::backend_traits<Backend>::diagnose(label) } -> std::same_as<ers::Diagnostic>;
        { ext::backend_traits<Backend>::retryable() } -> std::convertible_to<bool>;
    };
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::IsConnectable;
}
