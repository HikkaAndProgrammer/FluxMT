#pragma once

// std
#include <string>

// fluxmt
#include <fluxmt/drivers/psql/backend.hpp>
#include <fluxmt/drivers/psql/layouts.hpp>
#include <fluxmt/sql.hpp>

// export
#include <fluxmt/export.hpp>


namespace flux::impl::psql {
    using select_builder = QueryBuilder<backend, select_layout>;
    using insert_builder = QueryBuilder<backend, insert_layout>;
    using update_builder = QueryBuilder<backend, update_layout>;
    using delete_builder = QueryBuilder<backend, delete_layout>;
    using exists_builder = QueryBuilder<backend, exists_layout>;
}


namespace flux::psql::orm {
    using namespace flux::orm;

    FLUXMT_EXPORT impl::psql::select_builder select_from(std::string table, std::string alias = "");
    FLUXMT_EXPORT impl::psql::select_builder select();
    FLUXMT_EXPORT impl::psql::insert_builder insert_into(std::string table);
    FLUXMT_EXPORT impl::psql::update_builder update(std::string table);
    FLUXMT_EXPORT impl::psql::delete_builder delete_from(std::string table);
    FLUXMT_EXPORT impl::psql::exists_builder exists_in(std::string table);
}
