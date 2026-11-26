#include "fluxmt/query/query.hpp"

// std
#include <algorithm>

// fluxmt
#include <fluxmt/query/error.hpp>


// Implementation --------------------------------------------------------------

// Observers

bool flux::impl::Query::has(slot_ref slot) const {
    return std::ranges::any_of(_clauses, [&](const Clause& it) {
        return same_slot(it->slot(), slot);
    });
}

ers::Result<flux::impl::Layout> flux::impl::Query::layout(const Dialect& dialect) const {
    const auto base = _kind ? dialect.layout(_kind) : _fixed;

    if (base.empty()) {
        return ers::make_error(EQueryError::Unsupported, "'{}' statement in '{}'",
            _kind ? _kind->name : std::string_view("custom"), dialect.name());
    }

    Layout out(base);

    for (const auto& edit : _edits) {
        switch (edit.op) {
            case EEdit::After:
                ERS_TRY(out.insert_after(edit.anchor, edit.binding));
                break;

            case EEdit::Before:
                ERS_TRY(out.insert_before(edit.anchor, edit.binding));
                break;

            case EEdit::Append:
                ERS_TRY(out.append(edit.binding));
                break;
        }
    }

    return out;
}


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

ers::Status flux::impl::Query::insert_after(slot_ref anchor, const slot_info_t& binding) {
    return _add_edit(EEdit::After, anchor, binding);
}

ers::Status flux::impl::Query::insert_before(slot_ref anchor, const slot_info_t& binding) {
    return _add_edit(EEdit::Before, anchor, binding);
}

ers::Status flux::impl::Query::append(const slot_info_t& binding) {
    return _add_edit(EEdit::Append, nullptr, binding);
}

ers::Status flux::impl::Query::_add_edit(EEdit op, slot_ref anchor, const slot_info_t& binding) {
    if (!binding.slot)
        return ers::make_error(EQueryError::NullSlot, "slot");

    if (op != EEdit::Append && !anchor)
        return ers::make_error(EQueryError::NullSlot, "anchor");

    _edits.push_back(layout_edit_t { .op = op, .anchor = anchor, .binding = binding });

    return ers::ok;
}


// Executors

ers::Status flux::impl::Query::build(Context& ctx) const {
    const auto resolved = ERS_TRY(layout(ctx.dialect()));

    ERS_TRY(_validate(resolved, ctx.dialect()));

    for (const auto& binding : resolved.bindings())
        ERS_TRY(_render_slot(ctx, binding));

    return ers::ok;
}

ers::Result<flux::impl::statement_t> flux::impl::Query::to_statement(const Dialect& dialect) const {
    const auto ctx = dialect.make_context();
    ERS_TRY(build(*ctx));

    return std::move(*ctx).finish();
}


// Rendering

ers::Status flux::impl::Query::_render_slot(Context& ctx, const slot_info_t& binding) const {
    if (binding.lead_unless && has(binding.slot) && !has(binding.lead_unless))
        ctx.text += binding.lead;

    if (binding.renderer)
        return _render_custom(ctx, binding);

    bool empty = true;

    for (const auto& it : _clauses) {
        if (!same_slot(it->slot(), binding.slot))
            continue;

        ctx.text += empty ? binding.prefix : binding.separator;
        empty = false;

        ERS_TRY(it->render(ctx));
    }

    if (empty) {
        if (binding.fallback.empty())
            return ers::ok;

        ctx.text += binding.prefix;
        ctx.text += binding.fallback;
    }

    ctx.text += binding.suffix;

    return ers::ok;
}

ers::Status flux::impl::Query::_render_custom(Context& ctx, const slot_info_t& binding) const {
    std::vector<const Clause*> group;

    for (const auto& it : _clauses) {
        if (same_slot(it->slot(), binding.slot))
            group.push_back(&it);
    }

    if (group.empty()) {
        if (binding.fallback.empty())
            return ers::ok;

        ctx.text += binding.prefix;
        ctx.text += binding.fallback;
        ctx.text += binding.suffix;

        return ers::ok;
    }

    return binding.renderer(ctx, group);
}


// Validators

ers::Status flux::impl::Query::_validate(const Layout& layout, const Dialect& dialect) const {
    for (const auto& it : _clauses) {
        if (layout.contains(it->slot()))
            continue;

        if (_kind) {
            return ers::make_error(EQueryError::Unsupported, "'{}' in '{}' for '{}'",
                it->slot()->name, dialect.name(), _kind->name);
        }

        return ers::make_error(EQueryError::ForeignSlot, "'{}'", it->slot()->name);
    }

    for (const auto& binding : layout.bindings()) {
        if (!has(binding.slot))
            continue;

        for (const auto other : binding.excludes) {
            if (has(other)) {
                return ers::make_error(EQueryError::SlotConflict, "'{}' and '{}'",
                    binding.slot->name, other->name);
            }
        }
    }

    return ers::ok;
}


// Subqueries

ers::Status flux::impl::append_subquery(
    Context& ctx, const Query& sub,
    std::string_view open, std::string_view close
) {
    ctx.text += open;
    ERS_TRY(sub.build(ctx));

    ctx.text += close;

    return ers::ok;
}
