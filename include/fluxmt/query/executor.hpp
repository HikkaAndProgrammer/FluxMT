#pragma once

// std
#include <concepts>
#include <type_traits>

// ers
#include <erslib/core/type/result.hpp>

// fluxmt
#include <fluxmt/core/backend.hpp>
#include <fluxmt/core/tx.hpp>
#include <fluxmt/query/dialect.hpp>
#include <fluxmt/query/statement.hpp>


namespace flux::ext {
    template<typename NativeTx>
    struct executor_traits;
}


namespace flux::impl {
    template<typename NativeTx>
    using executor_backend_t = typename ext::executor_traits<NativeTx>::backend;

    template<typename NativeTx>
    using executor_result_t = typename ext::executor_traits<NativeTx>::result;


    template<typename T>
    concept IsExecutor = requires(T& tx, const statement_t& statement) {
        typename ext::executor_traits<T>::backend;
        typename ext::executor_traits<T>::result;

        requires IsBackend<executor_backend_t<T>>;
        requires IsNativeTransaction<T, executor_backend_t<T>>;

        { ext::backend_traits<executor_backend_t<T>>::dialect() } -> std::convertible_to<const Dialect&>;
        { ext::executor_traits<T>::exec(tx, statement) } -> std::same_as<executor_result_t<T>>;
    };


    template<typename T>
    concept IsBuilderSource = requires(const T& source) {
        typename T::backend;
        { source.to_statement() } -> std::same_as<ers::Result<statement_t>>;
    };

    template<typename T>
    concept IsStatementSource = std::same_as<std::remove_cvref_t<T>, statement_t>
        || IsBuilderSource<T>
        || requires(const T& source, const Dialect& dialect) {
            { source.to_statement(dialect) } -> std::same_as<ers::Result<statement_t>>;
        };
}


namespace flux::impl {
    template<IsExecutor NativeTx, IsStatementSource Source>
    ers::Result<statement_t> build_statement(const Source& source) {
        if constexpr (std::same_as<std::remove_cvref_t<Source>, statement_t>) {
            return source;
        } else if constexpr (IsBuilderSource<Source>) {
            static_assert(std::same_as<typename Source::backend, executor_backend_t<NativeTx>>,
                "the query was built for another backend than the transaction runs on");

            return source.to_statement();
        } else {
            return source.to_statement(ext::backend_traits<executor_backend_t<NativeTx>>::dialect());
        }
    }
}
