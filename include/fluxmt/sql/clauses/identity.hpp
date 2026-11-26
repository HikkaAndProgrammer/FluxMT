#pragma once

// std
#include <string>

// fluxmt
#include <fluxmt/query/clause.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::clauses {
    FLUXMT_EXPORT Clause identifier(slot_ref slot, std::string name, std::string alias = "");

    FLUXMT_EXPORT Clause from(std::string name, std::string alias = "");
}
