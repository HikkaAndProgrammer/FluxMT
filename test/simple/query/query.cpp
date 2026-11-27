// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// fluxmt
#include <fluxmt/query.hpp>

// test
#include "support/test_case.hpp"


// Details ---------------------------------------------------------------------

namespace {
    inline constexpr flux::query::slot_t key_slot { .name = "test.key", .arity = flux::query::EArity::Single };
    inline constexpr flux::query::slot_t value_slot { .name = "test.value", .arity = flux::query::EArity::Single };
    inline constexpr flux::query::slot_t nx_slot { .name = "test.nx", .arity = flux::query::EArity::Single };
    inline constexpr flux::query::slot_t ttl_slot { .name = "test.ttl", .arity = flux::query::EArity::Single };

    constexpr flux::query::slot_info_t set_layout[] = {
        { .slot = &key_slot, .prefix = "SET " },
        { .slot = &value_slot, .prefix = " " },
        { .slot = &nx_slot, .prefix = " NX" },
    };

    class ValueClause final : public flux::ext::IClause {
    public:
        ValueClause(flux::query::slot_ref slot, flux::Value value) :
            IClause(slot),
            _value(std::move(value)) {
        }

        ers::Status render(flux::ext::Context& ctx) const override {
            ctx.text += ctx.bind(_value);

            return ers::ok;
        }

    private:
        flux::Value _value;
    };


    class FlagClause final : public flux::ext::IClause {
    public:
        explicit FlagClause(flux::query::slot_ref slot) :
            IClause(slot) {
        }

        ers::Status render(flux::ext::Context&) const override {
            return ers::ok;
        }
    };


    class CommandDialect final : public flux::ext::Dialect {
    public:
        std::string_view name() const noexcept override { return "command"; }
        std::string placeholder(size_t index) const override { return "$" + std::to_string(index); }

        std::unique_ptr<flux::ext::Context> make_context() const override {
            return std::make_unique<flux::ext::Context>(*this);
        }
    };


    ers::Result<flux::statement_t> statement_of(const flux::Query& query) {
        static const CommandDialect dialect;

        return query.to_statement(dialect);
    }

    flux::Query set_query() {
        flux::Query q(set_layout);
        q |= flux::query::make_clause<ValueClause>(&key_slot, std::string("k"));
        q |= flux::query::make_clause<ValueClause>(&value_slot, 5);

        return q;
    }
}


// Queries ---------------------------------------------------------------------

FLUXMT_TEST_CASE("query renders a statement that is not SQL", "[query]") {
    auto q = set_query();
    q |= flux::query::make_clause<FlagClause>(&nx_slot);

    const auto r = statement_of(q);

    REQUIRE(r);
    CHECK(r->text == "SET $1 $2 NX");
    CHECK(r->params == std::vector<flux::Value>{ std::string("k"), 5 });

    CHECK(statement_of(set_query())->text == "SET $1 $2");
}

FLUXMT_TEST_CASE("query replaces a clause of a single slot", "[query]") {
    auto q = set_query();
    q |= flux::query::make_clause<ValueClause>(&value_slot, 7);

    const auto r = statement_of(q);

    REQUIRE(r);
    CHECK(r->text == "SET $1 $2");
    CHECK(r->params == std::vector<flux::Value>{ std::string("k"), 7 });
}

FLUXMT_TEST_CASE("query refuses what its layout does not have", "[query]") {
    auto q = set_query();
    q |= flux::query::make_clause<ValueClause>(&ttl_slot, 60);

    const auto slot = statement_of(q);

    REQUIRE_FALSE(slot);
    CHECK(slot.error().code() == flux::EQueryError::ForeignSlot);
    CHECK(slot.error().message().contains("command"));

    const auto empty = statement_of(flux::Query(flux::query::slot_info_view {}));

    REQUIRE_FALSE(empty);
    CHECK(empty.error().code() == flux::EQueryError::Unsupported);
}

FLUXMT_TEST_CASE("query layout is extended per query", "[query]") {
    auto q = set_query();

    REQUIRE(q.insert_after(&value_slot, { .slot = &ttl_slot, .prefix = " EX " }));
    q |= flux::query::make_clause<ValueClause>(&ttl_slot, 60);

    const auto r = statement_of(q);

    REQUIRE(r);
    CHECK(r->text == "SET $1 $2 EX $3");

    CHECK(statement_of(set_query())->text == "SET $1 $2");

    CHECK_FALSE(q.insert_after(nullptr, { .slot = &ttl_slot }));
    CHECK_FALSE(q.insert_after(&value_slot, { .slot = nullptr }));

    auto lost = set_query();
    REQUIRE(lost.insert_after(&nx_slot, { .slot = &key_slot }));

    const auto duplicate = statement_of(lost);

    REQUIRE_FALSE(duplicate);
    CHECK(duplicate.error().code() == flux::EQueryError::DuplicateSlot);
}

FLUXMT_TEST_CASE("query renders the layout it was given", "[query]") {
    static constexpr flux::query::slot_info_t own[] = {
        { .slot = &key_slot, .prefix = "GET " },
    };

    flux::Query q { flux::query::slot_info_view(own) };
    q |= flux::query::make_clause<ValueClause>(&key_slot, std::string("k"));

    CHECK(statement_of(q)->text == "GET $1");

    q |= flux::query::make_clause<ValueClause>(&value_slot, 1);

    const auto foreign = statement_of(q);

    REQUIRE_FALSE(foreign);
    CHECK(foreign.error().code() == flux::EQueryError::ForeignSlot);
}

FLUXMT_TEST_CASE("query writes a lead before a slot when another is empty", "[query]") {
    static constexpr flux::query::slot_info_t own[] = {
        { .slot = &key_slot, .prefix = "GET " },
        { .slot = &ttl_slot, .prefix = " TTL ", .lead_unless = &nx_slot, .lead = " NONX" },
    };

    flux::Query q { flux::query::slot_info_view(own) };
    q |= flux::query::make_clause<ValueClause>(&key_slot, std::string("k"));

    CHECK(statement_of(q)->text == "GET $1");

    q |= flux::query::make_clause<ValueClause>(&ttl_slot, 5);

    CHECK(statement_of(q)->text == "GET $1 NONX TTL $2");
}


// Typed clauses ---------------------------------------------------------------

FLUXMT_TEST_CASE("query takes typed slot clauses", "[query]") {
    using key_clause = flux::query::SlotClause<&key_slot>;
    using value_clause = flux::query::SlotClause<&value_slot>;
    using flag_clauses = flux::query::SlotClause<&nx_slot, true>;

    STATIC_REQUIRE(key_clause::slot == &key_slot);
    STATIC_REQUIRE(std::convertible_to<key_clause, flux::query::Clause>);
    STATIC_REQUIRE(!std::convertible_to<flag_clauses, flux::query::Clause>);
    STATIC_REQUIRE(std::convertible_to<flag_clauses, std::vector<flux::query::Clause>>);

    STATIC_REQUIRE(std::same_as<
        decltype(flux::query::when(true, std::declval<key_clause>())),
        flux::query::SlotClause<&key_slot, true>
    >);

    flux::Query q(set_layout);
    q |= key_clause(flux::query::make_clause<ValueClause>(&key_slot, std::string("k")));
    q |= flux::query::when(true, value_clause(flux::query::make_clause<ValueClause>(&value_slot, 5)));
    q |= flux::query::when(false, flag_clauses(std::vector<flux::query::Clause> {}));

    const auto r = statement_of(q);

    REQUIRE(r);
    CHECK(r->text == "SET $1 $2");
    CHECK(r->params == std::vector<flux::Value>{ std::string("k"), 5 });
}
