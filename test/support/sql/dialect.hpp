#pragma once

// std
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

// fluxmt
#include <fluxmt/drivers/shared.hpp>
#include <fluxmt/sql.hpp>


namespace test::anti {
    namespace shared = flux::impl::shared;

    inline constexpr flux::ext::SlotBinding select_layout[] = {
        shared::select_keyword,
        shared::distinct,
        shared::select_columns,
        shared::from,
        shared::join,
        shared::where,
        shared::group,
        shared::having,
        shared::order,
        shared::limit,
        shared::offset,
    };

    inline constexpr flux::ext::SlotBinding insert_layout[] = {
        shared::insert_into,
        shared::set_as_values,
        shared::insert_columns,
        shared::values,
        shared::conflict,
        shared::conflict_constraint,
        shared::conflict_action,
        shared::returning,
    };

    inline constexpr flux::ext::SlotBinding update_layout[] = {
        shared::update_target,
        shared::set,
        shared::where,
        shared::returning,
    };

    inline constexpr flux::ext::SlotBinding delete_layout[] = {
        shared::delete_from,
        shared::where,
        shared::returning,
    };

    inline constexpr flux::ext::SlotBinding exists_layout[] = {
        shared::exists_target,
        shared::where,
        shared::exists_alias,
    };

    inline constexpr auto layouts = std::to_array<flux::sql::layout_entry_t>({
        { &flux::kinds::select, select_layout },
        { &flux::kinds::insert, insert_layout },
        { &flux::kinds::update, update_layout },
        { &flux::kinds::erase,  delete_layout },
        { &flux::kinds::exists, exists_layout },
    });
}


namespace test::anti {
    inline constexpr flux::sql::syntax_t syntax {
        .cast     = flux::syntax::cast_function,
        .ilike    = flux::syntax::ilike_via_lower,
        .any      = flux::syntax::any_as_in_list,
        .excluded = flux::syntax::excluded_table,
    };
}


namespace test {
    class AntiDialect final : public flux::sql::Dialect {
    public:
        AntiDialect() noexcept : Dialect(anti::syntax) {}

        std::string_view name() const noexcept override { return "anti"; }
        std::string placeholder(size_t index) const override { return "?" + std::to_string(index); }
        bool is_reserved(std::string_view word) const noexcept override { return word == "item" || word == "group"; }

        flux::ext::LayoutView layout(flux::ext::KindRef kind) const noexcept override {
            return flux::ext::find_layout(anti::layouts, kind);
        }
    };
}
