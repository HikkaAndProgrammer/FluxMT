#pragma once

// std
#include <string>

// fluxmt
#include <fluxmt/sql.hpp>

// support
#include "backend.hpp"


namespace test::anti {
    using select_builder = flux::sql::QueryBuilder<anti_backend, select_layout>;
    using insert_builder = flux::sql::QueryBuilder<anti_backend, insert_layout>;
    using update_builder = flux::sql::QueryBuilder<anti_backend, update_layout>;
    using delete_builder = flux::sql::QueryBuilder<anti_backend, delete_layout>;
    using exists_builder = flux::sql::QueryBuilder<anti_backend, exists_layout>;
}


namespace test::anti::orm {
    using namespace flux::orm;

    select_builder select_from(std::string table, std::string alias = "");
    select_builder select();
    insert_builder insert_into(std::string table);
    update_builder update(std::string table);
    delete_builder delete_from(std::string table);
    exists_builder exists_in(std::string table);
}
