#pragma once

// Core ------------------------------------------------------------------------

#include <fluxmt/sql/query/clause.hpp>
#include <fluxmt/sql/query/kind.hpp>
#include <fluxmt/sql/query/layout.hpp>
#include <fluxmt/sql/query/query.hpp>
#include <fluxmt/sql/query/slot.hpp>


// Slots -----------------------------------------------------------------------

#include <fluxmt/sql/query/slots/column.hpp>
#include <fluxmt/sql/query/slots/conflict.hpp>
#include <fluxmt/sql/query/slots/distinct.hpp>
#include <fluxmt/sql/query/slots/from.hpp>
#include <fluxmt/sql/query/slots/group.hpp>
#include <fluxmt/sql/query/slots/having.hpp>
#include <fluxmt/sql/query/slots/head.hpp>
#include <fluxmt/sql/query/slots/join.hpp>
#include <fluxmt/sql/query/slots/limit.hpp>
#include <fluxmt/sql/query/slots/offset.hpp>
#include <fluxmt/sql/query/slots/order.hpp>
#include <fluxmt/sql/query/slots/returning.hpp>
#include <fluxmt/sql/query/slots/set.hpp>
#include <fluxmt/sql/query/slots/values.hpp>
#include <fluxmt/sql/query/slots/where.hpp>


// Clauses ---------------------------------------------------------------------

#include <fluxmt/sql/query/clauses/assign.hpp>
#include <fluxmt/sql/query/clauses/bound.hpp>
#include <fluxmt/sql/query/clauses/column.hpp>
#include <fluxmt/sql/query/clauses/conditional.hpp>
#include <fluxmt/sql/query/clauses/conflict.hpp>
#include <fluxmt/sql/query/clauses/distinct.hpp>
#include <fluxmt/sql/query/clauses/group.hpp>
#include <fluxmt/sql/query/clauses/identity.hpp>
#include <fluxmt/sql/query/clauses/join.hpp>
#include <fluxmt/sql/query/clauses/order.hpp>
#include <fluxmt/sql/query/clauses/raw.hpp>
#include <fluxmt/sql/query/clauses/returning.hpp>
#include <fluxmt/sql/query/clauses/values.hpp>
#include <fluxmt/sql/query/clauses/where.hpp>


// Layouts ---------------------------------------------------------------------

#include <fluxmt/sql/query/layouts/delete.hpp>
#include <fluxmt/sql/query/layouts/exists.hpp>
#include <fluxmt/sql/query/layouts/insert.hpp>
#include <fluxmt/sql/query/layouts/select.hpp>
#include <fluxmt/sql/query/layouts/update.hpp>
