# Targets

## Overview

| Feature                                    | psql    | sqlite       |
| ------------------------------------------ | ------- | ------------ |
| Client library                             | libpqxx | SQLite C API |
| Connection pool (`Pool<Backend>`)          | yes     | yes          |
| Typed builders (`<driver>::orm`)           | yes     | yes          |
| Compile-time refusal of clauses and syntax | yes     | yes          |


## Backend feature flags

| Flag           | psql | sqlite |
| -------------- | ---- | ------ |
| `Sql`          | Y    | Y      |
| `Transactions` | Y    | Y      |
| `Returning`    | Y    | Y      |
| `Upsert`       | Y    | Y      |
| `DistinctOn`   | Y    | N      |
| `NativeArrays` | Y    | N      |


## Connection

| Feature                               | psql                                   | sqlite                                           |
| ------------------------------------- | -------------------------------------- | ------------------------------------------------ |
| Options                               | host, port, dbname, username, password | path, read_only, foreign_keys, wal, busy_timeout |
| Default                               | localhost:5432                         | `:memory:`                                       |
| In-memory database                    | no                                     | yes*                                             |
| Liveness check of a pooled connection | yes                                    | yes                                              |

* Every connection of a pool has its own, a shared test needs a file


## Transactions

| Feature                                                   | psql                                 | sqlite                     |
| --------------------------------------------------------- | ------------------------------------ | -------------------------- |
| Read-only transaction                                     | yes (`read_transaction`)             | no*                        |
| Take the write lock at the start                          | n/a                                  | yes (`write_transaction`)  |
| Commit and rollback by `with_tx` and `try_with_tx`        | yes                                  | yes                        |
| Retry of a whole transaction (`tx_options_t::retries`)    | yes: serialization failure, deadlock | yes: busy, locked snapshot |
| Callable takes a native transaction, `Tx<...>` or nothing | yes                                  | yes                        |

* Open read-only through `read_only`


## SQL text

| Feature                                          | psql                                                        | sqlite                         |
| ------------------------------------------------ | ----------------------------------------------------------- | ------------------------------ |
| Placeholder                                      | `$1`                                                        | `?1`                           |
| `SELECT`, `INSERT`, `UPDATE`, `DELETE`, `EXISTS` | yes                                                         | yes                            |
| `WITH` (CTE) and `UNION`                         | yes                                                         | yes                            |
| `RETURNING`                                      | yes                                                         | yes                            |
| Upsert                                           | `ON CONFLICT ... DO UPDATE`                                 | `ON CONFLICT ... DO UPDATE`    |
| `ON CONFLICT ON CONSTRAINT name`                 | yes                                                         | no*                            |
| Reference to the new row in upsert               | `excluded.col`                                              | `excluded.col`                 |
| `DISTINCT ON`                                    | yes: `distinct_on(...)`, refused together with `distinct()` | no                             |
| `ILIKE`                                          | native operator                                             | `lower(a) LIKE lower(b)`       |
| Cast                                             | `x::type`                                                   | `CAST(x AS type)`              |
| `col = ANY(array)`                               | native                                                      | spread into `IN (?1, ?2, ...)` |
| `OFFSET` without `LIMIT`                         | plain `OFFSET`                                              | `LIMIT -1 OFFSET ?`            |
| `now()`                                          | `now()`                                                     | `CURRENT_TIMESTAMP`            |
| `greatest(a, b, ...)` and `least(...)`           | native                                                      | `max(a, b)` and `min(a, b)`    |
| Other function names                             | kept as written                                             | kept as written                |
| Native array type                                | yes                                                         | no                             |
| JSON and full text search                        | planned                                                     | not supported                  |

* Refused at compile time, or `ForeignSlot` through an erased query


## Parameters and values

| `flux::Value` alternative          | psql                                 | sqlite                             |
| ---------------------------------- | ------------------------------------ | ---------------------------------- |
| NULL                               | yes                                  | yes                                |
| bool                               | yes                                  | only as 0 or 1                     |
| i64                                | yes                                  | yes                                |
| u64                                | yes                                  | yes, max is i64                    |
| f64                                | yes                                  | yes                                |
| string                             | yes, borrowed for the call (`zview`) | yes                                |
| bytes                              | yes (`bytea`)                        | yes (blob)                         |
| timestamp, date, uuid              | as ISO text, the server casts        | stored as text                     |
| array (`value_array_t`)            | as one `{...}` text parameter        | `std::invalid_argument`, use `ANY` |
| custom (`custom_value_t`)          | its text                             | text                               |
| Parameter types sent to the server | untyped text                         | typed binding                      |


## Execution

| Feature                           | psql                           | sqlite                              |
| --------------------------------- | ------------------------------ | ----------------------------------- |
| `executor_traits` for             | every `pqxx::transaction_base` | every `sqlite::transaction_base`    |
| Result of a statement             | `pqxx::result`                 | `sqlite::result_t`                  |
| Affected rows                     | `result.affected_rows()`       | `affected`                          |
| Last inserted id                  | through `RETURNING`            | `last_insert_rowid`, or `RETURNING` |
| `with_query`, `try_with_query`    | yes                            | yes                                 |
| Reading rows by name, `get_as<T>` | `pqxx`                         | `Value`                             |


## Errors

| Error code (`EDbError`)            | psql                            | sqlite                                 |
| ---------------------------------- | ------------------------------- | -------------------------------------- |
| UniqueViolation                    | SQLSTATE `23505`                | unique, primary key, rowid             |
| ForeignKeyViolation                | `23503`                         | foreign key                            |
| CheckViolation                     | `23514`                         | check                                  |
| NotNullViolation                   | `23502`                         | not null                               |
| SerializationFailure               | `40001`                         | busy snapshot                          |
| Deadlock                           | `40P01`                         | n/a                                    |
| Busy                               | n/a                             | busy, locked                           |
| Timeout                            | `57014`                         | interrupt                              |
| ConnectionLost                     | class `08`                      | cannot open, I/O error, not a database |
| PoolTimeout                        | yes, from the pool              | yes, from the pool                     |
| Other                              | everything else                 | everything else                        |
| Retried by `tx_options_t::retries` | serialization failure, deadlock | busy, locked snapshot                  |
