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

export module block_iter;

import std;
import type_alias;
import catalog;
import block_column_iter;

namespace infinity {

export class BlockIter {
public:
    BlockIter(const BlockEntry *entry, const std::vector<SizeT> &column_ids) {
        std::vector<BlockColumnIter> column_iters;
        for (auto column_id : column_ids) {
            column_iters.emplace_back(entry->GetColumnDataByID(column_id), entry->row_count());
        }
        column_iters_ = column_iters;
    }

    std::optional<std::vector<const void *>> Next() {
        std::vector<const void *> rets;
        for (auto &column_iter : column_iters_) {
            auto ret = column_iter.Next();
            if (!ret) {
                return std::nullopt;
            }
            rets.emplace_back(*ret);
        }
        return rets;
    }

private:
    std::vector<BlockColumnIter> column_iters_{};
};

} // namespace infinity