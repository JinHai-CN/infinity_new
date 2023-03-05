//
// Created by jinhai on 23-3-5.
//

#include "drop_table_info.h"

namespace infinity {

String
DropTableInfo::ToString() const {
    std::stringstream ss;
    ss << "DROP TABLE: " << schema_name_ << "." << table_name_;
    if (this->conflict_type_ == ConflictType::kIgnore) {
        ss << " IgnoreIfExists";
    } else {
        ss << " ThrowErrorIfExists";
    }
    return ss.str();
}

}
