#pragma once

// std
#include <concepts>
#include <functional>
#include <string_view>
#include <type_traits>
#include <utility>

// ers
#include <erslib/core/trait/result.hpp>
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core/connectable.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    template<IsConnectable Backend, std::invocable Fn>
    auto try_invoke(Fn&& fn, std::string_view label = {}) -> ers::as_result_t<std::invoke_result_t<Fn>> {
        using std::string_view_literals::operator ""sv;
        using R = std::invoke_result_t<Fn>;

        try {
            if constexpr (std::is_void_v<R>) {
                std::invoke(std::forward<Fn>(fn));
                return ers::ok;
            } else {
                return std::invoke(std::forward<Fn>(fn));
            }
        } catch (...) {
            return ext::backend_traits<Backend>::diagnose(label.empty() ? "database call"sv : label);
        }
    }
}
