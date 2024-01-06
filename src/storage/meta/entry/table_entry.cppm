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

export module catalog:table_entry;

import :segment_entry;
import :block_entry;
import :table_index_meta;
import :base_entry;

import std;
import type_alias;
import parser;
import txn_store;
import buffer_manager;
import third_party;
import table_entry_type;
import block_index;
import data_access_state;
import status;

namespace infinity {

class DBEntry;
class IndexDef;
struct TableIndexEntry;
class IrsIndexEntry;
class TableMeta;

struct TableEntry : public BaseEntry {
    friend struct NewCatalog;

public:
    // for iterator unit test.
    explicit TableEntry() : BaseEntry(EntryType::kTable) {}

    explicit TableEntry(const std::shared_ptr<std::string> &db_entry_dir,
                        std::shared_ptr<std::string> table_collection_name,
                        const std::vector<std::shared_ptr<ColumnDef>> &columns,
                        TableEntryType table_entry_type,
                        TableMeta *table_meta,
                        u64 txn_id,
                        TxnTimeStamp begin_ts);

private:
    std::tuple<TableIndexEntry *, Status>
    CreateIndex(const std::shared_ptr<IndexDef> &index_def, ConflictType conflict_type, u64 txn_id, TxnTimeStamp begin_ts, TxnManager *txn_mgr);

    std::tuple<TableIndexEntry *, Status>
    DropIndex(const std::string &index_name, ConflictType conflict_type, u64 txn_id, TxnTimeStamp begin_ts, TxnManager *txn_mgr);

    std::tuple<TableIndexEntry *, Status> GetIndex(const std::string &index_name, u64 txn_id, TxnTimeStamp begin_ts);

    void RemoveIndexEntry(const std::string &index_name, u64 txn_id, TxnManager *txn_mgr);

    void CreateIndexFile(void *txn_store, TableIndexEntry *table_index_entry, TxnTimeStamp begin_ts, BufferManager *buffer_mgr, bool prepare);

    static void CommitCreateIndex(std::unordered_map<std::string, TxnIndexStore> &txn_indexes_store_);

    TableMeta *GetTableMeta() const { return table_meta_; }

    void Append(u64 txn_id, void *txn_store, BufferManager *buffer_mgr);

    void CommitAppend(u64 txn_id, TxnTimeStamp commit_ts, const AppendState *append_state_ptr);

    void RollbackAppend(u64 txn_id, TxnTimeStamp commit_ts, void *txn_store);

    Status Delete(u64 txn_id, TxnTimeStamp commit_ts, DeleteState &delete_state);

    void CommitDelete(u64 txn_id, TxnTimeStamp commit_ts, const DeleteState &append_state);

    Status RollbackDelete(u64 txn_id, DeleteState &append_state, BufferManager *buffer_mgr);

    Status ImportSegment(TxnTimeStamp commit_ts, std::shared_ptr<SegmentEntry> segment);

    static inline u32 GetNextSegmentID(TableEntry *table_entry) { return table_entry->next_segment_id_++; }

    static inline u32 GetMaxSegmentID(const TableEntry *table_entry) { return table_entry->next_segment_id_; }

    static SegmentEntry *GetSegmentByID(const TableEntry *table_entry, u32 seg_id);

public:
    // Getter

    const std::shared_ptr<std::string> &GetDBName() const;

    inline const std::shared_ptr<std::string> &GetTableName() const { return table_name_; }

    const BlockEntry *GetBlockEntryByID(u32 seg_id, u16 block_id) const;

    inline const ColumnDef *GetColumnDefByID(u64 column_id) const { return columns_[column_id].get(); }

    inline SizeT ColumnCount() const { return columns_.size(); }

    const std::shared_ptr<std::string> &TableEntryDir() const { return table_entry_dir_; }

    inline SizeT row_count() const { return row_count_; }

    inline TableEntryType EntryType() const { return table_entry_type_; }

    std::pair<SizeT, Status> GetSegmentRowCountBySegmentID(u32 seg_id);

    std::shared_ptr<BlockIndex> GetBlockIndex(u64 txn_id, TxnTimeStamp begin_ts);

    void GetFullTextAnalyzers(u64 txn_id, TxnTimeStamp begin_ts, std::shared_ptr<IrsIndexEntry> &irs_index_entry, std::map<std::string, std::string> &column2analyzer);

public:
    Json Serialize(TxnTimeStamp max_commit_ts, bool is_full_checkpoint);

    static std::unique_ptr<TableEntry> Deserialize(const Json &table_entry_json, TableMeta *table_meta, BufferManager *buffer_mgr);

    virtual void MergeFrom(BaseEntry &other);

public:
    u64 GetColumnIdByName(const std::string &column_name) const;

    std::map<u32, std::shared_ptr<SegmentEntry>> &segment_map() { return segment_map_; }

    std::unordered_map<std::string, std::unique_ptr<TableIndexMeta>> &index_meta_map() { return index_meta_map_; }

protected:
    std::unordered_map<std::string, u64> column_name2column_id_;

    std::shared_mutex rw_locker_{};

    std::shared_ptr<std::string> table_entry_dir_{};

    std::shared_ptr<std::string> table_name_{};

    std::vector<std::shared_ptr<ColumnDef>> columns_{};

    TableEntryType table_entry_type_{TableEntryType::kTableEntry};

    TableMeta *table_meta_{};

    // From data table
    std::atomic<SizeT> row_count_{};
    std::map<u32, std::shared_ptr<SegmentEntry>> segment_map_{};
    SegmentEntry *unsealed_segment_{};
    std::atomic<u32> next_segment_id_{};

    // Index definition
    std::unordered_map<std::string, std::unique_ptr<TableIndexMeta>> index_meta_map_{};
};

} // namespace infinity
