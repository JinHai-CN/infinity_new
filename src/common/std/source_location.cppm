module;
#include <experimental/source_location>
# 3 __FILE__ 1 3 // Enter "faked" system files since std is reserved module name
export module std:source_location;
export namespace std {
    using std::experimental::source_location;
}

