//
// Created by JinHai on 2022/10/30.
//

#include "common/types/internal_types.h"
#include "point_type.h"

namespace infinity {

// A circle is specified by a center point and radius
struct CircleType {
    PointType center;
    f64 radius;

    inline void
    Reset() {
        center.Reset();
        radius = 0;
    }
};

}

