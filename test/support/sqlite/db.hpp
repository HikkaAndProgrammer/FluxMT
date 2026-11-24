#pragma once

// std
#include <filesystem>

// fluxmt
#include <fluxmt/sqlite.hpp>


namespace test {
    class TempDb {
    public:
        // Member functions

        TempDb();
        ~TempDb();

        TempDb(const TempDb&) = delete;
        TempDb& operator=(const TempDb&) = delete;


        // Accessors

        flux::sqlite::options_t options() const;


    private:
        std::filesystem::path _path;
    };
}
