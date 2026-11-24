#include "fluxmt/drivers/sqlite/options.hpp"


bool flux::impl::sqlite::options_t::in_memory() const noexcept {
    return path.starts_with(":memory:")
        || path.contains("mode=memory");
}
