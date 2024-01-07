module;
#include <typeinfo>
# 3 __FILE__ 1 3 // Enter "faked" system files since std is reserved module name
export module std:type_info;
export namespace std {
    using std::type_info;
}
