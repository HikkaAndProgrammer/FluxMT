#pragma once

// pqxx
#include <pqxx/transaction>

// fluxmt
#include <fluxmt/core.hpp>
#include <fluxmt/drivers/psql/backend.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::psql {
    using Database = Pool<backend>;

    using Tx = impl::Tx<backend, pqxx::work>;
    using ReadTx = impl::Tx<backend, pqxx::read_transaction>;

    template<typename NativeTx>
    using TxOf = impl::Tx<backend, NativeTx>;
}
