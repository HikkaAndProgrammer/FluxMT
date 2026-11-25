#pragma once

// std
#include <string_view>


namespace test::live {
    // Runs "<FLUXMT_TEST_LIVEDB_COMMAND> up <target>" once per process; does nothing when the command is empty.
    void start_database(std::string_view target);
}
