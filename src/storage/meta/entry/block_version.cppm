// Copyright(C) 2023 InfiniFlow, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

module;

export module block_version;

import stl;

namespace infinity {

struct CreateField {
    //    CreateField(TxnTimeStamp create_ts, i32 row_count) : create_ts_(create_ts), row_count_(row_count) {}

    TxnTimeStamp create_ts_{};
    i64 row_count_{};

    bool operator==(const CreateField &rhs) const { return create_ts_ == rhs.create_ts_ && row_count_ == rhs.row_count_; }

    bool operator!=(const CreateField &rhs) const { return !(*this == rhs); }
};

export struct BlockVersion {
    constexpr static std::string_view PATH = "version";

    explicit BlockVersion(SizeT capacity) : deleted_(capacity, 0) {}
    bool operator==(const BlockVersion &rhs) const;
    bool operator!=(const BlockVersion &rhs) const { return !(*this == rhs); };
    i32 GetRowCount(TxnTimeStamp begin_ts);
    void LoadFromFile(const String &version_path);
    void SaveToFile(const String &version_path);

    void Cleanup(const String &version_path);

    Vector<CreateField> created_{}; // second field width is same as timestamp, otherwise Valgrind will issue BlockVersion::SaveToFile has
                                    // risk to write uninitialized buffer. (ts, rows)
    Vector<TxnTimeStamp> deleted_{};
};

} // namespace infinity
