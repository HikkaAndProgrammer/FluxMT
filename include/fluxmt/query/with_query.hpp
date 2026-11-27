#pragma once

// std
#include <string_view>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core/tx.hpp>
#include <fluxmt/core/tx_try.hpp>
#include <fluxmt/query/executor.hpp>


// Native transaction ----------------------------------------------------------

namespace flux::impl {
    template<IsExecutor NativeTx, IsStatementSource Source>
    executor_result_t<NativeTx> with_query(NativeTx& tx, const Source& source) {
        auto statement = build_statement<NativeTx>(source).or_throw();
        return ext::executor_traits<NativeTx>::exec(tx, statement);
    }

    template<IsBackend Backend, IsExecutor NativeTx, IsStatementSource Source>
    executor_result_t<NativeTx> with_query(Tx<Backend, NativeTx>& tx, const Source& source) {
        return with_query(tx.native(), source);
    }

    template<IsExecutor NativeTx, IsStatementSource Source>
    ers::Result<executor_result_t<NativeTx>> try_with_query(NativeTx& tx, const Source& source, std::string_view label = {}) {
        using R = ers::Result<executor_result_t<NativeTx>>;

        auto statement = build_statement<NativeTx>(source);
        if (!statement)
            return R(std::move(statement).extract_error());

        return try_invoke<executor_backend_t<NativeTx>>([&] {
            return ext::executor_traits<NativeTx>::exec(tx, *statement);
        }, label);
    }
}


// Pool ------------------------------------------------------------------------

namespace flux::impl {
    template<IsConnectable Backend, IsStatementSource Source>
        requires IsExecutor<typename ext::backend_traits<Backend>::default_tx>
    default_result_t<Backend> with_query(Pool<Backend>& pool, const Source& source, const tx_options_t& opts = {}) {
        using native = typename ext::backend_traits<Backend>::default_tx;

        auto statement = build_statement<native>(source).or_throw();

        return with_tx(pool, [&](native& tx) {
            return ext::executor_traits<native>::exec(tx, statement);
        }, opts);
    }

    template<IsConnectable Backend, IsStatementSource Source>
    default_result_t<Backend> with_query(Pool<Backend>& pool, const Source& source, std::string_view label) {
        return with_query(pool, source, tx_options_t { .label = label });
    }


    template<IsConnectable Backend, IsStatementSource Source>
        requires IsExecutor<typename ext::backend_traits<Backend>::default_tx>
    ers::Result<default_result_t<Backend>> try_with_query(Pool<Backend>& pool, const Source& source, const tx_options_t& opts = {}) {
        using native = typename ext::backend_traits<Backend>::default_tx;
        using R = ers::Result<executor_result_t<native>>;

        auto statement = build_statement<native>(source);
        if (!statement)
            return R(std::move(statement).extract_error());

        return try_with_tx(pool, [&](native& tx) {
            return ext::executor_traits<native>::exec(tx, *statement);
        }, opts);
    }

    template<IsConnectable Backend, IsStatementSource Source>
    ers::Result<default_result_t<Backend>> try_with_query(Pool<Backend>& pool, const Source& source, std::string_view label) {
        return try_with_query(pool, source, tx_options_t { .label = label });
    }
}


// Connection ------------------------------------------------------------------

namespace flux::impl {
    template<IsConnectable Backend, IsStatementSource Source>
        requires IsExecutor<typename ext::backend_traits<Backend>::default_tx>
    default_result_t<Backend> with_query(connection_t<Backend>& conn, const Source& source, const tx_options_t& opts = {}) {
        using native = typename ext::backend_traits<Backend>::default_tx;

        auto statement = build_statement<native>(source).or_throw();

        return with_tx<Backend>(conn, [&](native& tx) {
            return ext::executor_traits<native>::exec(tx, statement);
        }, opts);
    }

    template<IsConnectable Backend, IsStatementSource Source>
        requires IsExecutor<typename ext::backend_traits<Backend>::default_tx>
    ers::Result<default_result_t<Backend>> try_with_query(connection_t<Backend>& conn, const Source& source, const tx_options_t& opts = {}) {
        using native = typename ext::backend_traits<Backend>::default_tx;
        using R = ers::Result<default_result_t<Backend>>;

        auto statement = build_statement<native>(source);
        if (!statement)
            return R(std::move(statement).extract_error());

        return try_with_tx<Backend>(conn, [&](native& tx) {
            return ext::executor_traits<native>::exec(tx, *statement);
        }, opts);
    }
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::try_with_query;
    using impl::with_query;
}
