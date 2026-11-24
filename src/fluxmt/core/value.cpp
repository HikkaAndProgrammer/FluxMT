#include "fluxmt/core/value.hpp"


bool flux::Value::is_null() const noexcept {
    return std::holds_alternative<std::monostate>(_storage);
}
size_t flux::Value::index() const noexcept {
    return _storage.index();
}


bool flux::Value::operator==(const Value& other) const {
    return _storage == other._storage;
}
