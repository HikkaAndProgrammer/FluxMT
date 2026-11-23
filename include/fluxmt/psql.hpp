#pragma once

// Includes --------------------------------------------------------------------

#include <fluxmt/core.hpp>
#include <fluxmt/psql/backend.hpp>
#include <fluxmt/psql/connection.hpp>
#include <fluxmt/psql/error.hpp>
#include <fluxmt/psql/options.hpp>
#include <fluxmt/psql/params.hpp>
#include <fluxmt/psql/tx.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::psql {
    using impl::psql::backend;
    using impl::psql::options_t;

    using impl::psql::Database;
    using impl::psql::ReadTx;
    using impl::psql::Tx;
    using impl::psql::TxOf;

    using impl::pool_options_t;
    using impl::tx_options_t;

    using impl::try_with_conn;
    using impl::psql::try_with_tx;
    using impl::try_with_tx;
    using impl::psql::with_tx;
    using impl::with_conn;
    using impl::with_tx;
}
