module;
#include <ctime>
# 3 __FILE__ 1 3 // Enter "faked" system files since std is reserved module name
export module std:ctime;
export namespace std {
    using std::time;
}
