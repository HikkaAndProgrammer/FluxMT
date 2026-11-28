#pragma once

// fluxmt
#include <fluxmt/core.hpp>
#include <fluxmt/drivers/sqlite/backend.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::sqlite {
    using Database = Pool<backend>;

    using Tx = impl::Tx<backend, transaction>;
    using WriteTx = impl::Tx<backend, write_transaction>;

    template<typename NativeTx>
    using TxOf = impl::Tx<backend, NativeTx>;
}
