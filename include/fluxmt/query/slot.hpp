#pragma once

// std
#include <span>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/query/fwd.hpp>


namespace flux::impl {
    class Context;
}


namespace flux::impl {
    enum class EArity : bool {
        Single, Multi
    };

    using slot_renderer_t = ers::Status(*)(Context&, std::span<const Clause* const>);


    using slot_ref = const struct slot_t*;
    struct slot_t {
        std::string_view name;
        EArity arity;
    };


    struct slot_info_t {
        slot_ref slot = nullptr;

        std::string_view prefix;
        std::string_view separator;
        std::string_view suffix;
        std::string_view fallback;

        slot_renderer_t renderer = nullptr;

        std::span<const slot_ref> excludes;

        slot_ref lead_unless = nullptr;
        std::string_view lead;
    };

    using slot_info_view = std::span<const slot_info_t>;
    using slot_info_array = std::vector<slot_info_t>;


    constexpr bool same_slot(slot_ref lhs, slot_ref rhs) noexcept {
        return lhs && rhs && lhs->name == rhs->name;
    }
}
