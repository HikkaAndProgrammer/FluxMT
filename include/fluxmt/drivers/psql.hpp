#pragma once

// Includes --------------------------------------------------------------------

#include <fluxmt/core.hpp>
#include <fluxmt/drivers/psql/backend.hpp>
#include <fluxmt/drivers/psql/connection.hpp>
#include <fluxmt/drivers/psql/dialect.hpp>
#include <fluxmt/drivers/psql/error.hpp>
#include <fluxmt/drivers/psql/options.hpp>
#include <fluxmt/drivers/psql/orm.hpp>
#include <fluxmt/drivers/psql/params.hpp>
#include <fluxmt/drivers/psql/tx.hpp>


// Exports ---------------------------------------------------------------------

namespace flux::psql {
    using impl::psql::backend;
    using impl::psql::options_t;

    using impl::psql::Database;
    using impl::psql::ReadTx;
    using impl::psql::Tx;
    using impl::psql::TxOf;


    using impl::psql::dialect;
    using impl::psql::postgres_dialect;
    using impl::psql::reserved_words;
}
