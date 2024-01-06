module;
#include <unordered_set>
# 3 __FILE__ 1 3 // Enter "faked" system files since std is reserved module name
export module std:unordered_set;
export namespace std {
    using std::unordered_set;
}
