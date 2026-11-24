#pragma once

// std
#include <stdexcept>
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/diagnostic.hpp>

// fluxmt
#include <fluxmt/core/error.hpp>

// export
#include <fluxmt/export.hpp>


struct sqlite3;


namespace flux::impl::sqlite {
    class FLUXMT_EXPORT error : public std::runtime_error {
    public:
        error(int extended_code, const std::string& message);

        int code() const noexcept;


    private:
        int _code;
    };
}


namespace flux::impl::sqlite {
    FLUXMT_EXPORT error last_error(::sqlite3* db, int fallback = 1);
    FLUXMT_EXPORT EDbError classify(int extended_code) noexcept;

    FLUXMT_EXPORT ers::Diagnostic diagnose(std::string_view label);
    FLUXMT_EXPORT bool retryable() noexcept;
}
