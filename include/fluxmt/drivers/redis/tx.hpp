#pragma once

// fluxmt
#include <fluxmt/core.hpp>
#include <fluxmt/drivers/redis/backend.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::redis {
    using Database = Pool<backend>;

    using Tx = impl::Tx<backend, transaction>;

    template<typename NativeTx>
    using TxOf = impl::Tx<backend, NativeTx>;
}
