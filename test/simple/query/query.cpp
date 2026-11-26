// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <array>
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
    inline constexpr flux::ext::statement_kind_t set_kind { .name = "test.set" };
    inline constexpr flux::ext::statement_kind_t other_kind { .name = "test.other" };

    inline constexpr flux::ext::slot_t key_slot { .name = "test.key", .arity = flux::ext::EArity::Single };
    inline constexpr flux::ext::slot_t value_slot { .name = "test.value", .arity = flux::ext::EArity::Single };
    inline constexpr flux::ext::slot_t nx_slot { .name = "test.nx", .arity = flux::ext::EArity::Single };
    inline constexpr flux::ext::slot_t ttl_slot { .name = "test.ttl", .arity = flux::ext::EArity::Single };

    constexpr flux::ext::slot_info_t set_layout[] = {
        { .slot = &key_slot, .prefix = "SET " },
        { .slot = &value_slot, .prefix = " " },
        { .slot = &nx_slot, .prefix = " NX" },
    };

    constexpr auto table = std::to_array<flux::query::layout_entry_t>({
        { &set_kind, set_layout },
    });


    class ValueClause final : public flux::ext::IClause {
    public:
        ValueClause(flux::ext::slot_ref slot, flux::Value value) :
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
        explicit FlagClause(flux::ext::slot_ref slot) :
            IClause(slot) {
        }

        ers::Status render(flux::ext::Context&) const override {
            return ers::ok;
        }
    };


    class CommandDialect final : public flux::query::Dialect {
    public:
        std::string_view name() const noexcept override { return "command"; }
        std::string placeholder(size_t index) const override { return "$" + std::to_string(index); }

        flux::query::slot_info_view layout(flux::query::kind_ref kind) const noexcept override {
            return flux::query::find_layout(table, kind);
        }

        std::unique_ptr<flux::query::Context> make_context() const override {
            return std::make_unique<flux::query::Context>(*this);
        }
    };


    ers::Result<flux::query::statement_t> statement_of(const flux::query::Query& query) {
        static const CommandDialect dialect;

        return query.to_statement(dialect);
    }

    flux::query::Query set_query() {
        flux::query::Query q(&set_kind);
        q |= flux::ext::make_clause<ValueClause>(&key_slot, std::string("k"));
        q |= flux::ext::make_clause<ValueClause>(&value_slot, 5);

        return q;
    }
}


// Queries ---------------------------------------------------------------------

FLUXMT_TEST_CASE("query renders a statement that is not SQL", "[query]") {
    auto q = set_query();
    q |= flux::ext::make_clause<FlagClause>(&nx_slot);

    const auto r = statement_of(q);

    REQUIRE(r);
    CHECK(r->text == "SET $1 $2 NX");
    CHECK(r->params == std::vector<flux::Value>{ std::string("k"), 5 });

    CHECK(statement_of(set_query())->text == "SET $1 $2");
}

FLUXMT_TEST_CASE("query replaces a clause of a single slot", "[query]") {
    auto q = set_query();
    q |= flux::ext::make_clause<ValueClause>(&value_slot, 7);

    const auto r = statement_of(q);

    REQUIRE(r);
    CHECK(r->text == "SET $1 $2");
    CHECK(r->params == std::vector<flux::Value>{ std::string("k"), 7 });
}

FLUXMT_TEST_CASE("query refuses what the layout of the dialect does not have", "[query]") {
    auto q = set_query();
    q |= flux::ext::make_clause<ValueClause>(&ttl_slot, 60);

    const auto slot = statement_of(q);

    REQUIRE_FALSE(slot);
    CHECK(slot.error().code() == flux::EQueryError::Unsupported);
    CHECK(slot.error().message().contains("command"));

    const auto kind = statement_of(flux::query::Query(&other_kind));

    REQUIRE_FALSE(kind);
    CHECK(kind.error().code() == flux::EQueryError::Unsupported);
}

FLUXMT_TEST_CASE("query layout is extended per query", "[query]") {
    auto q = set_query();

    REQUIRE(q.insert_after(&value_slot, { .slot = &ttl_slot, .prefix = " EX " }));
    q |= flux::ext::make_clause<ValueClause>(&ttl_slot, 60);

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

FLUXMT_TEST_CASE("query with a layout of its own does not ask the dialect", "[query]") {
    static constexpr flux::ext::slot_info_t own[] = {
        { .slot = &key_slot, .prefix = "GET " },
    };

    flux::query::Query q { flux::ext::slot_info_view(own) };
    q |= flux::ext::make_clause<ValueClause>(&key_slot, std::string("k"));

    CHECK(statement_of(q)->text == "GET $1");

    q |= flux::ext::make_clause<ValueClause>(&value_slot, 1);

    const auto foreign = statement_of(q);

    REQUIRE_FALSE(foreign);
    CHECK(foreign.error().code() == flux::EQueryError::ForeignSlot);
}

FLUXMT_TEST_CASE("query writes a lead before a slot when another is empty", "[query]") {
    static constexpr flux::ext::slot_info_t own[] = {
        { .slot = &key_slot, .prefix = "GET " },
        { .slot = &ttl_slot, .prefix = " TTL ", .lead_unless = &nx_slot, .lead = " NONX" },
    };

    flux::query::Query q { flux::ext::slot_info_view(own) };
    q |= flux::ext::make_clause<ValueClause>(&key_slot, std::string("k"));

    CHECK(statement_of(q)->text == "GET $1");

    q |= flux::ext::make_clause<ValueClause>(&ttl_slot, 5);

    CHECK(statement_of(q)->text == "GET $1 NONX TTL $2");
}
