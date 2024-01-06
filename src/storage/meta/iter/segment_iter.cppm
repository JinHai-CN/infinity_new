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

export module segment_iter;

import std;
import type_alias;
import catalog;
import block_iter;

namespace infinity {

export class SegmentIter {
public:
    SegmentIter(const SegmentEntry *entry, std::shared_ptr<std::vector<SizeT>> column_ids) : entry_(entry), block_idx_(0), column_ids_(column_ids) {
        const auto &block_entries = entry->block_entries();
        if (block_entries.empty()) {
            block_iter_ = std::nullopt;
        } else {
            block_iter_ = BlockIter(block_entries[block_idx_].get(), *column_ids);
        }
    }

    std::optional<std::vector<const void *>> Next() {
        if (!block_iter_.has_value()) {
            return std::nullopt;
        }
        if (auto ret = block_iter_->Next(); ret) {
            return ret;
        }
        block_idx_++;

        const auto &block_entries = entry_->block_entries();
        if (block_idx_ >= block_entries.size()) {
            block_iter_ = std::nullopt;
            return std::nullopt;
        }
        block_iter_ = BlockIter(block_entries[block_idx_].get(), *column_ids_);
        return Next();
    }

private:
    const SegmentEntry *entry_;
    SizeT block_idx_;
    std::optional<BlockIter> block_iter_;

    std::shared_ptr<std::vector<SizeT>> column_ids_;
};

export template <typename DataType>
class OneColumnIterator {
public:
    OneColumnIterator(const SegmentEntry *entry, SizeT column_id)
        : segment_iter_(entry, std::make_shared<std::vector<SizeT>>(std::vector<SizeT>{column_id})) {}

    std::optional<const DataType *> Next() {
        if (auto ret = segment_iter_.Next(); ret) {
            return reinterpret_cast<const DataType *>((*ret)[0]);
        }
        return std::nullopt;
    }

private:
    SegmentIter segment_iter_;
};

} // namespace infinity