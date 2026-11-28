#pragma once

// std
#include <string_view>
#include <utility>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core/try_invoke.hpp>
#include <fluxmt/core/tx.hpp>
#include <fluxmt/executors/with_tx.hpp>
#include <fluxmt/query/executor.hpp>


namespace flux::impl {
    struct with_query_fn {
        template<IsExecutor NativeTx, IsStatementSource Source>
        executor_result_t<NativeTx> operator()(NativeTx& tx, const Source& source) const {
            auto statement = build_statement<NativeTx>(source).or_throw();
            return ext::executor_traits<NativeTx>::exec(tx, statement);
        }

        template<IsBackend Backend, IsExecutor NativeTx, IsStatementSource Source>
        executor_result_t<NativeTx> operator()(Tx<Backend, NativeTx>& tx, const Source& source) const {
            return (*this)(tx.native(), source);
        }

        template<IsConnectable Backend, IsStatementSource Source>
            requires IsExecutor<typename ext::backend_traits<Backend>::default_tx>
        default_result_t<Backend> operator()(
            Pool<Backend>& pool, const Source& source, const tx_options_t& opts = {}
        ) const {
            using native = typename ext::backend_traits<Backend>::default_tx;

            auto statement = build_statement<native>(source).or_throw();

            return with_tx(pool, [&](native& tx) {
                return ext::executor_traits<native>::exec(tx, statement);
            }, opts);
        }

        template<IsConnectable Backend, IsStatementSource Source>
        default_result_t<Backend> operator()(Pool<Backend>& pool, const Source& source, std::string_view label) const {
            return (*this)(pool, source, tx_options_t { .label = label });
        }
    };

    inline constexpr with_query_fn with_query {};
}


namespace flux::impl {
    struct try_with_query_fn {
        template<IsExecutor NativeTx, IsStatementSource Source>
        ers::Result<executor_result_t<NativeTx>> operator()(
            NativeTx& tx, const Source& source, std::string_view label = {}
        ) const {
            using R = ers::Result<executor_result_t<NativeTx>>;

            auto statement = build_statement<NativeTx>(source);
            if (!statement)
                return R(std::move(statement).extract_error());

            return try_invoke<executor_backend_t<NativeTx>>([&] {
                return ext::executor_traits<NativeTx>::exec(tx, *statement);
            }, label);
        }

        template<IsConnectable Backend, IsStatementSource Source>
            requires IsExecutor<typename ext::backend_traits<Backend>::default_tx>
        ers::Result<default_result_t<Backend>> operator()(
            Pool<Backend>& pool, const Source& source, const tx_options_t& opts = {}
        ) const {
            using native = typename ext::backend_traits<Backend>::default_tx;
            using R = ers::Result<default_result_t<Backend>>;

            auto statement = build_statement<native>(source);
            if (!statement)
                return R(std::move(statement).extract_error());

            return try_with_tx(pool, [&](native& tx) {
                return with_query(tx, *statement);
            }, opts);
        }

        template<IsConnectable Backend, IsStatementSource Source>
        ers::Result<default_result_t<Backend>> operator()(
            Pool<Backend>& pool, const Source& source, std::string_view label
        ) const {
            return (*this)(pool, source, tx_options_t { .label = label });
        }
    };

    inline constexpr try_with_query_fn try_with_query {};
}


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::try_with_query;
    using impl::with_query;
}
