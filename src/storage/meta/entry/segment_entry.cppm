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

export module catalog:segment_entry;

import :block_entry;
import :column_index_entry;
import :segment_column_index_entry;
import :base_entry;

import std;
import type_alias;
import default_values;
import third_party;
import buffer_manager;
import index_base;
import data_access_state;
import parser;
import index_file_worker;
import infinity_exception;

namespace infinity {

class TxnTableStore;
struct TableEntry;

struct SegmentEntry : public BaseEntry {
    friend struct TableEntry;

public:
    explicit SegmentEntry(const TableEntry *table_entry);

    static std::shared_ptr<SegmentEntry> MakeNewSegmentEntry(const TableEntry *table_entry, u32 segment_id, BufferManager *buffer_mgr);

    static std::shared_ptr<SegmentEntry>
    MakeReplaySegmentEntry(const TableEntry *table_entry, u32 segment_id, std::shared_ptr<std::string> segment_dir, TxnTimeStamp commit_ts);

    static std::unique_ptr<CreateIndexParam> GetCreateIndexParam(SizeT seg_row_count, const IndexBase *index_base, const ColumnDef *column_def);

    Json Serialize(TxnTimeStamp max_commit_ts, bool is_full_checkpoint);

    static std::shared_ptr<SegmentEntry> Deserialize(const Json &table_entry_json, TableEntry *table_entry, BufferManager *buffer_mgr);

    void MergeFrom(infinity::BaseEntry &other) override;

public:
    const std::string &DirPath() { return *segment_dir_; }

    inline const std::vector<std::shared_ptr<BlockEntry>> &block_entries() const { return block_entries_; }

    inline TxnTimeStamp min_row_ts() const { return min_row_ts_; }

    inline u32 segment_id() const { return segment_id_; }

    inline const TableEntry *GetTableEntry() const { return table_entry_; }

    BlockEntry *GetBlockEntryByID(u16 block_id) const;

    inline const std::shared_ptr<std::string> &segment_dir() const { return segment_dir_; }

    inline SizeT row_count() const { return row_count_; }

    int Room();

public:
    // Used in WAL replay & Physical Import
    inline void AppendBlockEntry(std::unique_ptr<BlockEntry> block_entry) { block_entries_.emplace_back(std::move(block_entry)); }

    inline void IncreaseRowCount(SizeT increased_row_count) { row_count_ += increased_row_count; }

public:
    // Used in PhysicalImport
    inline BlockEntry *GetLastEntry() const {
        if (block_entries_.empty()) {
            Error<StorageException>("No any block entries");
        }
        return block_entries_.back().get();
    }

protected:
    u64 AppendData(u64 txn_id, AppendState *append_state_ptr, BufferManager *buffer_mgr);

    void DeleteData(u64 txn_id, TxnTimeStamp commit_ts, const std::unordered_map<u16, std::vector<RowID>> &block_row_hashmap);

    std::shared_ptr<SegmentColumnIndexEntry> CreateIndexFile(ColumnIndexEntry *column_index_entry,
                                                       std::shared_ptr<ColumnDef> column_def,
                                                       TxnTimeStamp create_ts,
                                                       BufferManager *buffer_mgr,
                                                       TxnTableStore *txn_store,
                                                       bool prepare);

    void CommitAppend(u64 txn_id, TxnTimeStamp commit_ts, u16 block_id, u16 start_pos, u16 row_count);

    void CommitDelete(u64 txn_id, TxnTimeStamp commit_ts, const std::unordered_map<u16, std::vector<RowID>> &block_row_hashmap);

private:
    static std::shared_ptr<std::string> DetermineSegmentDir(const std::string &parent_dir, u32 seg_id);

protected:
    std::shared_mutex rw_locker_{};

    const TableEntry *table_entry_{};

    std::shared_ptr<std::string> segment_dir_{};
    u32 segment_id_{};

    SizeT row_count_{};
    SizeT row_capacity_{};
    u64 column_count_{};

    TxnTimeStamp min_row_ts_{0}; // Indicate the commit_ts which create this SegmentEntry
    TxnTimeStamp max_row_ts_{0}; // Indicate the max commit_ts which create/update/delete data inside this SegmentEntry

    std::vector<std::shared_ptr<BlockEntry>> block_entries_{};
};

} // namespace infinity
