// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <algorithm>
#include <iterator>
#include <set>
#include <string>

// pqxx
#include <pqxx/pqxx>

// fluxmt
#include <fluxmt/drivers/psql.hpp>

// test
#include "support/drivers/psql/live.hpp"
#include "support/test_case.hpp"


namespace {
    std::string join(const std::set<std::string>& words) {
        std::string out;

        for (const auto& word : words)
            out += word + ' ';

        return out;
    }
}


FLUXMT_TEST_CASE("psql reserved words match the server list", "[psql]") {
    test::live::Sandbox sandbox;

    const auto server = flux::with_tx(sandbox.db(), [](pqxx::work& tx) {
        std::set<std::string> words;

        for (const auto& row : tx.exec("SELECT word FROM pg_get_keywords() WHERE catcode IN ('R', 'T')"))
            words.insert(row[0].as<std::string>());

        return words;
    }, "keywords");

    std::set<std::string> ours;
    for (const auto word : flux::psql::reserved_words())
        ours.emplace(word);

    std::set<std::string> missing;
    std::set<std::string> extra;

    std::ranges::set_difference(server, ours, std::inserter(missing, missing.end()));
    std::ranges::set_difference(ours, server, std::inserter(extra, extra.end()));

    INFO("the server reserves, our list does not: " << join(missing));
    INFO("our list reserves, the server does not: " << join(extra));

    CHECK(missing.empty());
    CHECK(extra.empty());
}
