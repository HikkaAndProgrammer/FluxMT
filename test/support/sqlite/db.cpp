#include "support/sqlite/db.hpp"

// std
#include <format>
#include <random>
#include <system_error>


// Implementation --------------------------------------------------------------

test::TempDb::TempDb() {
    std::random_device rd;
    _path = std::filesystem::temp_directory_path() / std::format("fluxmt_{:08x}{:08x}.db", rd(), rd());
}

test::TempDb::~TempDb() {
    std::error_code ec;

    for (const char* suffix : { "", "-wal", "-shm", "-journal" })
        std::filesystem::remove(_path.string() + suffix, ec);
}

flux::sqlite::options_t test::TempDb::options() const {
    flux::sqlite::options_t out;
    out.path = _path.string();
    return out;
}
