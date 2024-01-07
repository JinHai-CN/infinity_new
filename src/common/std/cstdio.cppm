//
// Created by jinhai on 24-1-6.
//


module;
#include <cstdio>
# 3 __FILE__ 1 3 // Enter "faked" system files since std is reserved module name
export module std:cstdio;
export namespace std {
using std::rename;
}