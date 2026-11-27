#include "fluxmt/drivers/psql/params.hpp"

// std
#include <concepts>
#include <format>
#include <type_traits>

// ers
#include <erslib/core/convert/number.hpp>
#include <erslib/core/convert/pg_time.hpp>


// Details ---------------------------------------------------------------------

namespace {
    std::string bytes_text(const flux::impl::value_bytes_t& bytes) {
        std::string out = "\\x";

        for (auto b : bytes)
            out += std::format("{:02x}", static_cast<unsigned>(b));

        return out;
    }

    void quote_element(std::string& out, std::string_view text) {
        out += '"';

        for (char c : text) {
            if (c == '"' || c == '\\')
                out += '\\';

            out += c;
        }

        out += '"';
    }
}


// Implementation --------------------------------------------------------------

std::string flux::impl::psql::to_text(const Value& value) {
    return value.visit([](const auto& v) -> std::string {
        using T = std::remove_cvref_t<decltype(v)>;

        if constexpr (std::same_as<T, std::monostate>)
            return "NULL";
        else if constexpr (std::same_as<T, bool>)
            return v ? "true" : "false";
        else if constexpr (std::same_as<T, ers::i64> || std::same_as<T, ers::u64>)
            return std::to_string(v);
        else if constexpr (std::same_as<T, ers::f64>)
            return std::format("{}", v);
        else if constexpr (std::same_as<T, std::string>)
            return v;
        else if constexpr (std::same_as<T, value_bytes_t>)
            return bytes_text(v);
        else if constexpr (std::same_as<T, ers::timestamp_t>)
            return ers::timestamp_to_iso8601(v);
        else if constexpr (std::same_as<T, ers::date_t>)
            return ers::date_to_iso8601(v);
        else if constexpr (std::same_as<T, ers::uuid>)
            return ers::uuid_to_string(v);
        else if constexpr (std::same_as<T, value_array_t>) {
            std::string out = "{";
            bool first = true;

            for (const auto& e : v) {
                if (!first)
                    out += ',';
                first = false;

                if (e.is_null())
                    out += "NULL";
                else if (e.template get_if<value_array_t>())
                    out += to_text(e);
                else
                    quote_element(out, to_text(e));
            }

            return out += "}";
        } else if constexpr (std::same_as<T, custom_value_t>)
            return v.text;

        std::unreachable();
    });
}

void flux::impl::psql::append(pqxx::params& out, const Value& value) {
    value.visit([&](const auto& v) {
        using T = std::remove_cvref_t<decltype(v)>;

        if constexpr (std::same_as<T, std::monostate>)
            out.append();
        else if constexpr (std::same_as<T, bool>
            || std::same_as<T, ers::i64> || std::same_as<T, ers::u64>
            || std::same_as<T, ers::f64>)
            out.append(v);
        else if constexpr (std::same_as<T, std::string>)
            out.append(pqxx::zview(v));
        else if constexpr (std::same_as<T, value_bytes_t>)
            out.append(pqxx::bytes_view { v.data(), v.size() });
        else
            out.append(to_text(Value(v)));
    });
}

pqxx::params flux::impl::psql::make_params(std::span<const Value> values) {
    pqxx::params out;

    for (const auto& v : values)
        append(out, v);

    return out;
}
