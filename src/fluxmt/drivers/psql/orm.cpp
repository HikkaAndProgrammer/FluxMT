#include "fluxmt/drivers/psql/orm.hpp"

// std
#include <utility>


// Implementation --------------------------------------------------------------

namespace flux::psql::orm {
    impl::psql::select_builder select_from(std::string table, std::string alias) {
        return impl::psql::select_builder() | from(std::move(table), std::move(alias));
    }

    impl::psql::select_builder select() {
        return impl::psql::select_builder();
    }

    impl::psql::insert_builder insert_into(std::string table) {
        return impl::psql::insert_builder() | insert_target(std::move(table));
    }

    impl::psql::update_builder update(std::string table) {
        return impl::psql::update_builder() | update_target(std::move(table));
    }

    impl::psql::delete_builder delete_from(std::string table) {
        return impl::psql::delete_builder() | delete_target(std::move(table));
    }

    impl::psql::exists_builder exists_in(std::string table) {
        return impl::psql::exists_builder() | exists_target(std::move(table));
    }
}
