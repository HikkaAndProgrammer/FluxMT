#pragma once

// Includes --------------------------------------------------------------------

#include <fluxmt/core.hpp>
#include <fluxmt/drivers/redis/args.hpp>
#include <fluxmt/drivers/redis/backend.hpp>
#include <fluxmt/drivers/redis/connection.hpp>
#include <fluxmt/drivers/redis/error.hpp>
#include <fluxmt/drivers/redis/options.hpp>
#include <fluxmt/drivers/redis/transaction.hpp>
#include <fluxmt/drivers/redis/tx.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::redis {
    using impl::redis::backend;
    using impl::redis::connection;
    using impl::redis::error;
    using impl::redis::options_t;

    using impl::redis::transaction;

    using impl::redis::Database;
    using impl::redis::Tx;
    using impl::redis::TxOf;

    using impl::pool_options_t;
    using impl::tx_options_t;

    using impl::try_with_conn;
    using impl::redis::try_with_tx;
    using impl::try_with_tx;
    using impl::redis::with_tx;
    using impl::with_conn;
    using impl::with_tx;
}
