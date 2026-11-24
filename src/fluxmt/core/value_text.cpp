#include "fluxmt/core/value_text.hpp"

// std
#include <chrono>
#include <format>


// Implementation --------------------------------------------------------------

std::string flux::impl::to_text(const ers::date_t& value) {
    return std::format("{:%F}", value);
}

std::string flux::impl::to_text(const ers::timestamp_t& value) {
    return std::format("{:%F %T}", std::chrono::floor<std::chrono::microseconds>(value));
}

std::string flux::impl::to_text(const ers::uuid& value) {
    return ers::uuid_to_string(value);
}
