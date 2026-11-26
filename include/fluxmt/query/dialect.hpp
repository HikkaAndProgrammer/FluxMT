#pragma once

// std
#include <cstddef>
#include <memory>
#include <span>
#include <string>
#include <string_view>

// fluxmt
#include <fluxmt/query/kind.hpp>
#include <fluxmt/query/slot.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl {
    class Context;
}


namespace flux::impl {
    struct layout_entry_t {
        kind_ref kind;
        slot_info_view layout;
    };

    constexpr slot_info_view find_layout(std::span<const layout_entry_t> table, kind_ref kind) noexcept {
        for (const auto& it : table) {
            if (same_kind(it.kind, kind))
                return it.layout;
        }

        return {};
    }
}


namespace flux::impl {
    class FLUXMT_EXPORT Dialect {
    public:
        // Member functions

        virtual ~Dialect() = default;


        // Accessors

        virtual std::string_view name() const noexcept = 0;
        virtual std::string placeholder(size_t index) const = 0;
        virtual slot_info_view layout(kind_ref kind) const noexcept = 0;

        virtual std::unique_ptr<Context> make_context() const = 0;
    };
}
