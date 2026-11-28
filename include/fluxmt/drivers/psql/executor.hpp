#pragma once

// pqxx
#include <pqxx/result>
#include <pqxx/transaction>

// fluxmt
#include <fluxmt/drivers/psql/backend.hpp>
#include <fluxmt/query/executor.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::psql {
    FLUXMT_EXPORT pqxx::result execute(pqxx::transaction_base& tx, const statement_t& statement);
}


template<std::derived_from<pqxx::transaction_base> NativeTx>
struct flux::ext::executor_traits<NativeTx> {
    using backend = impl::psql::backend;
    using result = pqxx::result;

    static result exec(NativeTx& tx, const impl::statement_t& statement) {
        return impl::psql::execute(tx, statement);
    }
};
