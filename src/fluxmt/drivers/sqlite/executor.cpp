#include "fluxmt/drivers/sqlite/executor.hpp"


flux::impl::sqlite::result_t flux::impl::sqlite::execute(transaction_base& tx, const statement_t& statement) {
    return tx.exec(statement.text, statement.params);
}
