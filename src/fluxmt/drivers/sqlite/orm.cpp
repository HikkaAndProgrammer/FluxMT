#include "fluxmt/drivers/sqlite/orm.hpp"

// std
#include <utility>


// Implementation --------------------------------------------------------------

namespace flux::sqlite::orm {
    impl::sqlite::select_builder select_from(std::string table, std::string alias) {
        return impl::sqlite::select_builder() | from(std::move(table), std::move(alias));
    }

    impl::sqlite::select_builder select() {
        return impl::sqlite::select_builder();
    }

    impl::sqlite::insert_builder insert_into(std::string table) {
        return impl::sqlite::insert_builder() | insert_target(std::move(table));
    }

    impl::sqlite::update_builder update(std::string table) {
        return impl::sqlite::update_builder() | update_target(std::move(table));
    }

    impl::sqlite::delete_builder delete_from(std::string table) {
        return impl::sqlite::delete_builder() | delete_target(std::move(table));
    }

    impl::sqlite::exists_builder exists_in(std::string table) {
        return impl::sqlite::exists_builder() | exists_target(std::move(table));
    }
}
