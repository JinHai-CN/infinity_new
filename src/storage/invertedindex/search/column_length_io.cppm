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

export module column_length_io;

import stl;
import index_defines;
import internal_types;

namespace infinity {
export class ColumnLengthWriter {};

export class ColumnLengthReader {
public:
    ColumnLengthReader() = default;
    ~ColumnLengthReader() = default;

    u32 GetColumnLength(u64 column_id, RowID doc_id) { return 0; }
};
} // namespace infinity