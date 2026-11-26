// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <array>
#include <cstddef>
#include <string>
#include <string_view>

// fluxmt
#include <fluxmt/drivers/shared.hpp>
#include <fluxmt/sql.hpp>

// test
#include "support/sql/dialect.hpp"
#include "support/test_case.hpp"


using namespace flux::orm;


// Details ---------------------------------------------------------------------

namespace {
    namespace shared = flux::impl::shared;

    inline constexpr flux::ext::fn_t<"forbidden"> forbidden {};


    inline constexpr flux::ext::StatementKind truncate_kind { .name = "test.truncate" };

    inline constexpr flux::ext::Slot truncate_target {
        .name  = "test.truncate_target",
        .arity = flux::ext::EArity::Single,
    };

    constexpr flux::ext::SlotBinding hook_where {
        .slot      = &flux::slots::where,
        .prefix    = "\n/* before where */\nWHERE ",
        .separator = " AND ",
    };

    constexpr flux::ext::SlotBinding hook_limit {
        .slot   = &flux::slots::limit,
        .prefix = "\nFETCH FIRST ",
        .suffix = " ROWS ONLY",
    };

    constexpr flux::ext::SlotBinding select_layout[] = {
        shared::select_keyword,
        shared::select_columns,
        shared::from,
        hook_where,
        shared::order,
        hook_limit,
    };

    constexpr flux::ext::SlotBinding insert_layout[] = {
        shared::insert_into,
        shared::set_as_values,
        shared::insert_columns,
        shared::values,
    };

    constexpr flux::ext::SlotBinding truncate_layout[] = {
        { .slot = &truncate_target, .prefix = "TRUNCATE " },
    };

    constexpr auto layouts = std::to_array<flux::sql::layout_entry_t>({
        { &flux::kinds::select, select_layout },
        { &flux::kinds::insert, insert_layout },
        { &truncate_kind, truncate_layout },
    });


    constexpr flux::sql::function_rule_t now_keyword {
        .name      = "now",
        .max_arity = 0,
        .spelling  = { .name = "CURRENT_TIMESTAMP", .call = false },
    };

    constexpr flux::sql::function_rule_t greatest_max {
        .name     = "greatest",
        .spelling = { .name = "max" },
    };

    constexpr flux::sql::function_rules_t hook_functions { now_keyword, greatest_max };

    // What no rule says
    constexpr flux::sql::function_spelling_t spell_function(std::string_view name, size_t) noexcept {
        if (name == "forbidden")
            return { .supported = false, .reason = "no arguments, one or many" };

        return {};
    }

    constexpr flux::sql::syntax_t hook_syntax {
        .cast      = flux::syntax::cast_function,
        .ilike     = flux::syntax::ilike_via_lower,
        .any       = flux::syntax::any_as_in_list,
        .excluded  = flux::syntax::excluded_table,
        .functions = hook_functions,
        .function  = &spell_function,
    };
}
