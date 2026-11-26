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


namespace flux::impl::redis {
    class FLUXMT_EXPORT error : public std::runtime_error {
    public:
        error(EDbError kind, const std::string& message);

        EDbError kind() const noexcept;


    private:
        EDbError _kind;
    };
}


namespace flux::impl::redis {
    FLUXMT_EXPORT EDbError classify_reply(std::string_view text) noexcept;

    FLUXMT_EXPORT EDbError classify_transport(int hiredis_error) noexcept;

    FLUXMT_EXPORT ers::Diagnostic diagnose(std::string_view label);
    FLUXMT_EXPORT bool retryable() noexcept;
}
