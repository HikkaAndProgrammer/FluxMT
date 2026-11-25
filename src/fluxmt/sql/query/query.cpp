#include "fluxmt/sql/query/query.hpp"

// std
#include <algorithm>

// fluxmt
#include <fluxmt/sql/error.hpp>


// Implementation --------------------------------------------------------------

// Modifiers

void flux::impl::Query::add(Clause clause) {
    if (clause->slot()->arity == EArity::Single) {
        std::erase_if(_clauses, [&](const Clause& it) {
            return same_slot(it->slot(), clause->slot());
        });
    }

    _clauses.push_back(std::move(clause));
}

void flux::impl::Query::add(std::vector<Clause> clauses) {
    for (auto& it : clauses)
        add(std::move(it));
}

void flux::impl::Query::add(WithCte with_cte) {
    _ctes.push_back(std::move(with_cte));
}

void flux::impl::Query::add(WithUnion with_union) {
    _unions.push_back(std::move(with_union));
}


// Executors

ers::Status flux::impl::Query::build(Context& ctx) const {
    ERS_TRY(_validate());

    if (!_ctes.empty()) {
        const bool recursive = std::ranges::any_of(_ctes, [](const WithCte& c) { return c.recursive; });

        ctx.query += recursive ? "WITH RECURSIVE " : "WITH ";

        bool first = true;
        for (const auto& cte : _ctes) {
            if (!first)
                ctx.query += ",\n";
            first = false;

            ERS_TRY(ctx.append_identifier(cte.name));

            ctx.query += " AS (\n";
            ERS_TRY(cte.definition.build(ctx));

            ctx.query += "\n)";
        }

        ctx.query += "\n";
    }

    for (const auto& binding : _layout.bindings())
        ERS_TRY(_render_slot(ctx, binding));

    for (const auto& u : _unions) {
        ctx.query += u.all ? "\nUNION ALL\n" : "\nUNION\n";
        ERS_TRY(u.other.build(ctx));
    }

    return ers::ok;
}

ers::Result<flux::impl::Statement> flux::impl::Query::to_statement(const Dialect& dialect) const {
    Context ctx(dialect);
    ERS_TRY(build(ctx));

    return std::move(ctx).finish();
}


// Rendering

ers::Status flux::impl::Query::_render_slot(Context& ctx, const SlotBinding& binding) const {
    if (binding.renderer)
        return _render_custom(ctx, binding);

    bool empty = true;

    for (const auto& it : _clauses) {
        if (!same_slot(it->slot(), binding.slot))
            continue;

        ctx.query += empty ? binding.prefix : binding.separator;
        empty = false;

        ERS_TRY(it->render(ctx));
    }

    if (empty) {
        if (binding.fallback.empty())
            return ers::ok;

        ctx.query += binding.prefix;
        ctx.query += binding.fallback;
    }

    ctx.query += binding.suffix;

    return ers::ok;
}

ers::Status flux::impl::Query::_render_custom(Context& ctx, const SlotBinding& binding) const {
    std::vector<const Clause*> group;

    for (const auto& it : _clauses) {
        if (same_slot(it->slot(), binding.slot))
            group.push_back(&it);
    }

    if (group.empty()) {
        if (binding.fallback.empty())
            return ers::ok;

        ctx.query += binding.prefix;
        ctx.query += binding.fallback;
        ctx.query += binding.suffix;

        return ers::ok;
    }

    return binding.renderer(ctx, group);
}


// Validators

ers::Status flux::impl::Query::_validate() const {
    for (const auto& it : _clauses) {
        if (!_layout.contains(it->slot()))
            return ers::make_error(ESqlError::ForeignSlot, "'{}'", it->slot()->name);
    }

    for (const auto& binding : _layout.bindings()) {
        if (!_has(binding.slot))
            continue;

        for (const auto other : binding.excludes) {
            if (_has(other)) {
                return ers::make_error(ESqlError::SlotConflict, "'{}' and '{}'",
                    binding.slot->name, other->name);
            }
        }
    }

    return ers::ok;
}

bool flux::impl::Query::_has(SlotRef slot) const {
    return std::ranges::any_of(_clauses, [&](const Clause& it) {
        return same_slot(it->slot(), slot);
    });
}


// Statement-level parts

flux::impl::WithCte flux::impl::clauses::with_cte(std::string name, Query definition, bool recursive) {
    return WithCte {
        .definition = std::move(definition),
        .name       = std::move(name),
        .recursive  = recursive
    };
}

flux::impl::WithUnion flux::impl::clauses::union_with(Query other, bool all) {
    return WithUnion {
        .other = std::move(other),
        .all   = all
    };
}


// Subqueries

ers::Status flux::impl::append_subquery(
    Context& ctx, const Query& sub,
    std::string_view open, std::string_view close
) {
    ctx.query += open;
    ERS_TRY(sub.build(ctx));

    ctx.query += close;

    return ers::ok;
}
