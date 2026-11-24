#pragma once

// std
#include <chrono>
#include <string>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/time.hpp>


namespace flux::impl::redis {
    struct options_t {
        std::string host = "127.0.0.1";
        ers::u16 port = 6379;
        std::string unix_socket;

        std::string username;
        std::string password;
        std::string client_name;
        int db = 0;

        ers::ms_t connect_timeout = std::chrono::seconds(5);
        ers::ms_t command_timeout = std::chrono::seconds(5);
    };
}
