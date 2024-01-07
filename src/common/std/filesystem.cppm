//
// Created by jinhai on 24-1-6.
//

module;

#include <filesystem>
# 3 __FILE__ 1 3 // Enter "faked" system files since std is reserved module name
export module std:filesystem;

export namespace std::filesystem {
    using std::filesystem::directory_entry;
    using std::filesystem::file_size;
    using std::filesystem::path;
    using std::filesystem::remove;
    using std::filesystem::exists;
    using std::filesystem::create_directories;
    using std::filesystem::remove_all;
    using std::filesystem::directory_iterator;
    using std::filesystem::is_directory;
    using std::filesystem::absolute;
    using std::filesystem::recursive_directory_iterator;
    using std::filesystem::is_regular_file;
    using std::filesystem::file_size;
}
