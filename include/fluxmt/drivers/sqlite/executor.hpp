#pragma once

// fluxmt
#include <fluxmt/drivers/sqlite/backend.hpp>
#include <fluxmt/drivers/sqlite/transaction.hpp>
#include <fluxmt/query/executor.hpp>


namespace flux::impl::sqlite {
    result_t execute(transaction_base& tx, const statement_t& statement);
}


template<std::derived_from<flux::impl::sqlite::transaction_base> NativeTx>
struct flux::ext::executor_traits<NativeTx> {
    using backend = impl::sqlite::backend;
    using result = impl::sqlite::result_t;

    static result exec(NativeTx& tx, const impl::statement_t& statement) {
        return impl::sqlite::execute(tx, statement);
    }
};
