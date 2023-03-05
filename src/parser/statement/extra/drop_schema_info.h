//
// Created by jinhai on 23-2-24.
//

#include "extra_ddl_info.h"

namespace infinity {

class DropSchemaInfo : public ExtraDDLInfo {
public:
    explicit
    DropSchemaInfo() : ExtraDDLInfo(DDLType::kSchema) {}

    [[nodiscard]] String
    ToString() const final;

    String schema_name_;
};

}
