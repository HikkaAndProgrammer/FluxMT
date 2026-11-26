#pragma once

// std
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/sql/query/clause.hpp>


namespace flux::impl::clauses {
    inline std::vector<Clause> when(bool condition, Clause clause) {
        std::vector<Clause> out;

        if (condition)
            out.push_back(std::move(clause));

        return out;
    }

    inline std::vector<Clause> when(bool condition, std::vector<Clause> clauses) {
        if (!condition)
            return {};

        return clauses;
    }

    template<typename Fn>
        requires std::invocable<Fn>
    std::vector<Clause> when(bool condition, Fn&& make) {
        if (!condition)
            return {};

        using result_t = std::remove_cvref_t<std::invoke_result_t<Fn>>;

        if constexpr (std::same_as<result_t, std::vector<Clause>>) {
            return std::forward<Fn>(make)();
        } else {
            std::vector<Clause> out;
            out.push_back(std::forward<Fn>(make)());

            return out;
        }
    }
}
