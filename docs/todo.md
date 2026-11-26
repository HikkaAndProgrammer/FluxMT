# TODO

## psql

### std::string and pqxx::zview

psql parameters: `append(pqxx::params&, const Value&)` in `src/fluxmt/drivers/psql/params.cpp` copies a `std::string` value with `out.append(v)`.
Use the `zview` overload instead (`out.append(pqxx::zview(v))`) so the string is borrowed, 'cause `statement_t` outlives the call.
