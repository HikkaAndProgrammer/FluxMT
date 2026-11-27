#pragma once

// Includes --------------------------------------------------------------------

#include <fluxmt/core.hpp>
#include <fluxmt/drivers/sqlite/backend.hpp>
#include <fluxmt/drivers/sqlite/connection.hpp>
#include <fluxmt/drivers/sqlite/dialect.hpp>
#include <fluxmt/drivers/sqlite/error.hpp>
#include <fluxmt/drivers/sqlite/executor.hpp>
#include <fluxmt/drivers/sqlite/options.hpp>
#include <fluxmt/drivers/sqlite/orm.hpp>
#include <fluxmt/drivers/sqlite/params.hpp>
#include <fluxmt/drivers/sqlite/transaction.hpp>
#include <fluxmt/drivers/sqlite/tx.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::sqlite {
    using impl::sqlite::backend;
    using impl::sqlite::connection;
    using impl::sqlite::error;
    using impl::sqlite::options_t;
    using impl::sqlite::result_t;
    using impl::sqlite::row_t;

    using impl::sqlite::transaction_base;
    using impl::sqlite::transaction;
    using impl::sqlite::write_transaction;

    using impl::sqlite::Database;
    using impl::sqlite::Tx;
    using impl::sqlite::TxOf;
    using impl::sqlite::WriteTx;


    using impl::sqlite::dialect;
    using impl::sqlite::reserved_words;
    using impl::sqlite::sqlite_dialect;
}
