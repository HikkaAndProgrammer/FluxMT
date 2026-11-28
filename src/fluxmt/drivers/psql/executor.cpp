#include "fluxmt/drivers/psql/executor.hpp"

// fluxmt
#include <fluxmt/drivers/psql/params.hpp>


pqxx::result flux::impl::psql::execute(pqxx::transaction_base& tx, const statement_t& statement) {
    return tx.exec(pqxx::zview(statement.text), make_params(statement.params));
}
