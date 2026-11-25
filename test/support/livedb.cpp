#include "support/livedb.hpp"

// std
#include <cstdlib>
#include <format>
#include <mutex>
#include <print>
#include <set>
#include <string>


#ifndef FLUXMT_TEST_LIVEDB_COMMAND_DEFAULT
#define FLUXMT_TEST_LIVEDB_COMMAND_DEFAULT ""
#endif


// Implementation --------------------------------------------------------------

void test::live::start_database(std::string_view target) {
    static std::mutex mutex;
    static std::set<std::string, std::less<>> started;

    const char* env = std::getenv("FLUXMT_TEST_LIVEDB_COMMAND");
    std::string command = env && *env ? env : FLUXMT_TEST_LIVEDB_COMMAND_DEFAULT;

    if (command.empty())
        return;

    std::scoped_lock lock(mutex);

    if (!started.emplace(target).second)
        return;

    if (std::system(std::format("{} up {}", command, target).c_str()) != 0)
        std::println(stderr, "livedb: '{} up {}' failed", command, target);
}
