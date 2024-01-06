module;
#include <optional>
# 3 __FILE__ 1 3 // Enter "faked" system files since std is reserved module name
export module std:optional;
export namespace std {
    using std::optional;
    using std::nullopt;
}
