#include "fluxmt/sql/clauses/with.hpp"

// std
#include <cstddef>
#include <utility>

// fluxmt
#include <fluxmt/sql/context.hpp>
#include <fluxmt/sql/slots/with.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    class CteClause final : public IClause {
    public:
        CteClause(std::string name, Query definition, bool recursive) :
            IClause(&slots::with),
            _name(std::move(name)),
            _definition(std::move(definition)),
            _recursive(recursive) {
        }

        bool recursive() const noexcept { return _recursive; }

        ers::Status render(Context& base) const override {
            auto& ctx = sql_of(base);

            ERS_TRY(ctx.append_identifier(_name));

            ctx.text += " AS (\n";
            ERS_TRY(_definition.build(ctx));

            ctx.text += "\n)";

            return ers::ok;
        }

    private:
        std::string _name;
        Query _definition;
        bool _recursive;
    };


    class UnionClause final : public IClause {
    public:
        UnionClause(Query other, bool all) :
            IClause(&slots::set_union),
            _other(std::move(other)),
            _all(all) {
        }

        ers::Status render(Context& ctx) const override {
            ctx.text += _all ? "\nUNION ALL\n" : "\nUNION\n";

            return _other.build(ctx);
        }

    private:
        Query _other;
        bool _all;
    };
}


// Implementation --------------------------------------------------------------

ers::Status flux::impl::render_with(Context& ctx, std::span<const Clause* const> group) {
    bool recursive = false;

    for (const auto* clause : group) {
        if (const auto* cte = dynamic_cast<const CteClause*>(&**clause))
            recursive = recursive || cte->recursive();
    }

    ctx.text += recursive ? "WITH RECURSIVE " : "WITH ";

    for (size_t i = 0; i < group.size(); ++i) {
        if (i != 0)
            ctx.text += ",\n";

        ERS_TRY((*group[i])->render(ctx));
    }

    ctx.text += "\n";

    return ers::ok;
}


// Shortcuts

Clause flux::impl::clauses::with_cte(std::string name, Query definition, bool recursive) {
    return make_clause<CteClause>(std::move(name), std::move(definition), recursive);
}

Clause flux::impl::clauses::union_with(Query other, bool all) {
    return make_clause<UnionClause>(std::move(other), all);
}
