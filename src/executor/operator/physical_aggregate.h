//
// Created by JinHai on 2022/7/28.
//

#pragma once

#include <utility>

#include "executor/physical_operator.h"
#include "expression/base_expression.h"

namespace infinity {

class PhysicalAggregate : public PhysicalOperator{
public:
    explicit PhysicalAggregate(u64 id,
                               SharedPtr<PhysicalOperator> left,
                               Vector<SharedPtr<BaseExpression>> groups,
                               Vector<SharedPtr<BaseExpression>> aggregates)
                               : PhysicalOperator(PhysicalOperatorType::kAggregate, std::move(left), nullptr, id),
                               groups_(std::move(groups)),
                               aggregates_(std::move(aggregates))
                               {}

    ~PhysicalAggregate() override = default;

    void
    Init() override;

    void
    Execute(SharedPtr<QueryContext>& query_context) override;

    Vector<SharedPtr<BaseExpression>> groups_{};
    Vector<SharedPtr<BaseExpression>> aggregates_{};
};

}

