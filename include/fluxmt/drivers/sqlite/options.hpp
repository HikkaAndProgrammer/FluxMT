#pragma once

// std
#include <chrono>
#include <string>

// ers
#include <erslib/core/type/time.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::sqlite {
    struct FLUXMT_EXPORT options_t {
        std::string path = ":memory:";
        bool read_only = false;
        bool foreign_keys = true;
        bool wal = true;
        ers::ms_t busy_timeout = std::chrono::seconds(5);


        bool in_memory() const noexcept;
    };
}
