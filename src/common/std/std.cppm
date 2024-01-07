# 1 __FILE__ 1 3 // Enter "faked" system files since std is reserved module name
module;

#include <algorithm>
#include <cctype>

export module std;

export import :string;
export import :iostream;
//export import :numeric;
export import :vector;
export import :thread;
//export import :chrono;
export import :cmath;
export import :map;
export import :mutex;
export import :atomic;
export import :filesystem;
//export import :random;
//export import :cstdio;
//export import :fstream;
export import :memory;
//export import :exception;
//export import :stdexcept;
export import :utility;
//export import :algorithm;
export import :array;
//export import :functional;
//export import :type_traits;
//export import :cstdint;
//export import :iterator;
export import :list;
export import :condition_variable;
//export import :queue;
//export import :cstdlib;
export import :tuple;
//export import :initializer_list;
//export import :limits;
//export import :coroutine;
export import :unordered_map;
export import :unordered_set;
export import :optional;
export import :move;
export import :system_error;
export import :cstring;
export import :ctime;
export import :cstdio;

export namespace infinity {

void ToUpper(std::string & str) { std::transform(str.begin(), str.end(), str.begin(), ::toupper); }
int32_t ToUpper(int c) { return ::toupper(c); }

void ToLower(std::string & str) { std::transform(str.begin(), str.end(), str.begin(), ::tolower); }
int32_t ToLower(int c) { return ::tolower(c); }

}

