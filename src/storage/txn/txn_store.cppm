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

export module txn_store;

import std;
import type_alias;
import parser;
import data_access_state;
// import catalog;

namespace infinity {

class Txn;
struct TableIndexEntry;
struct IrsIndexEntry;
struct TableEntry;
struct SegmentEntry;
class DataBlock;
class SegmentColumnIndexEntry;

struct TxnSegmentIndexStore {
    std::unordered_map<u32, std::shared_ptr<SegmentColumnIndexEntry>> index_entry_map_{};
};

export struct TxnIndexStore {
public:
    explicit TxnIndexStore(TableIndexEntry *table_index_entry);
    TxnIndexStore() = default;

    TableIndexEntry *const table_index_entry_{};

    //    std::vector<ColumnIndexEntry*> column_index_entry_{};
    //
    IrsIndexEntry *irs_index_entry_{};

    std::unordered_map<u64, std::unordered_map<u32, std::shared_ptr<SegmentColumnIndexEntry>>> index_entry_map_{}; // column_id -> segment_id -> segment_column_index_entry
};

export class TxnTableStore {
public:
    explicit inline TxnTableStore(TableEntry *table_entry, Txn *txn) : table_entry_(table_entry), txn_(txn) {}

    std::unique_ptr<std::string> Append(const std::shared_ptr<DataBlock> &input_block);

    std::unique_ptr<std::string> Import(const std::shared_ptr<SegmentEntry> &segment);

    std::unique_ptr<std::string> CreateIndexFile(TableIndexEntry *table_index_entry, u64 column_id, u32 segment_id, std::shared_ptr<SegmentColumnIndexEntry> index);

    std::unique_ptr<std::string> Delete(const std::vector<RowID> &row_ids);

    void Scan(std::shared_ptr<DataBlock> &output_block);

    void Rollback();

    void PrepareCommit();

    void Commit() const;

public:
    std::vector<std::shared_ptr<DataBlock>> blocks_{};
    std::vector<std::shared_ptr<SegmentEntry>> uncommitted_segments_{};
    std::unordered_map<std::string, TxnIndexStore> txn_indexes_store_{};
    std::unique_ptr<AppendState> append_state_{};
    DeleteState delete_state_{};

    SizeT current_block_id_{0};

    TableEntry *table_entry_{};
    Txn *txn_{};
};

} // namespace infinity
