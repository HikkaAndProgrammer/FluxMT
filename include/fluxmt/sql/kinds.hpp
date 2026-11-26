#pragma once

// fluxmt
#include <fluxmt/query/kind.hpp>


namespace flux::impl::kinds {
    inline constexpr statement_kind_t select { .name = "flux.select" };
    inline constexpr statement_kind_t insert { .name = "flux.insert" };
    inline constexpr statement_kind_t update { .name = "flux.update" };
    inline constexpr statement_kind_t erase  { .name = "flux.delete" };
    inline constexpr statement_kind_t exists { .name = "flux.exists" };
}
