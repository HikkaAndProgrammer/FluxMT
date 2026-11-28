#include "support/sql/orm.hpp"

// std
#include <utility>


// Implementation --------------------------------------------------------------

namespace test::anti::orm {
    select_builder select_from(std::string table, std::string alias) {
        return select_builder() | from(std::move(table), std::move(alias));
    }

    select_builder select() {
        return select_builder();
    }

    insert_builder insert_into(std::string table) {
        return insert_builder() | insert_target(std::move(table));
    }

    update_builder update(std::string table) {
        return update_builder() | update_target(std::move(table));
    }

    delete_builder delete_from(std::string table) {
        return delete_builder() | delete_target(std::move(table));
    }

    exists_builder exists_in(std::string table) {
        return exists_builder() | exists_target(std::move(table));
    }
}
