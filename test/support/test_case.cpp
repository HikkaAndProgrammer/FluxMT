#include "support/test_case.hpp"

// std
#include <unordered_set>


const char* test::reduce_name(std::string_view name) {
    std::string reduced;
    bool pending = false;

    for (char c : name) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
            if (pending && !reduced.empty())
                reduced += '_';
            pending = false;
            reduced += c;
        } else
            pending = true;
    }


    static std::unordered_set<std::string> pool;
    return pool.insert(std::move(reduced)).first->c_str();
}
