#pragma once

// std
#include <memory>




namespace flux::impl {
    class IExprNode;
    using ExprNodePtr = std::shared_ptr<const IExprNode>;

    class Expr;
    struct Aliased;

    class Query;
}
