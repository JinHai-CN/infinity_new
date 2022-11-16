//
// Created by JinHai on 2022/10/28.
//

#pragma once

#include "common/types/internal_types.h"

namespace infinity {

struct DateTimeType {
    i32 date{};
    i32 time{};

    inline void
    Reset() {
        date = 0;
        time = 0;
    }
};

}
