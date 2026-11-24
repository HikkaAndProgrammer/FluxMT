#pragma once

// std
#include <span>
#include <string>

// pqxx
#include <pqxx/params>

// fluxmt
#include <fluxmt/core/value.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::psql {
    FLUXMT_EXPORT std::string to_text(const Value& value);

    FLUXMT_EXPORT void append(pqxx::params& out, const Value& value);
    FLUXMT_EXPORT pqxx::params make_params(std::span<const Value> values);
}
