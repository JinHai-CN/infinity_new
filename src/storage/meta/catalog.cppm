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

export module catalog;
export import :db_meta;
export import :db_entry;
export import :table_meta;
export import :table_entry;
export import :view_meta;
export import :view_entry;
export import :segment_entry;
export import :block_entry;
export import :block_column_entry;
export import :table_index_meta;
export import :table_index_entry;
export import :column_index_entry;
export import :segment_column_index_entry;
export import :irs_index_entry;
export import :base_entry;
import :base_meta;

import std;
import type_alias;
import parser;
import table_def;
import function;
import function_set;
import table_function;
import special_function;
import third_party;
import buffer_manager;
import profiler;
import status;
import default_values;
import table_detail;
import index_def;
import txn_store;
import data_access_state;

namespace infinity {

class ProfileHistory {
private:
    std::mutex lock_{};
    std::vector<std::shared_ptr<QueryProfiler>> queue{};
    SizeT front{};
    SizeT rear{};
    SizeT max_size{};

public:
    explicit ProfileHistory(SizeT size) {
        max_size = size + 1;
        queue.resize(max_size);
        front = 0;
        rear = 0;
    }

    void Enqueue(std::shared_ptr<QueryProfiler> &&profiler) {
        std::unique_lock<std::mutex> lk(lock_);
        if ((rear + 1) % max_size == front) {
            return;
        }
        queue[rear] = profiler;
        rear = (rear + 1) % max_size;
    }

    QueryProfiler *GetElement(SizeT index) {
        std::unique_lock<std::mutex> lk(lock_);

        // FIXME: Bug, unsigned integer: index will always >= 0
        if (index < 0 || index >= (rear - front + max_size) % max_size) {
            return nullptr;
        }
        SizeT actualIndex = (front + index) % max_size;
        return queue[actualIndex].get();
    }

    std::vector<std::shared_ptr<QueryProfiler>> GetElements() {
        std::vector<std::shared_ptr<QueryProfiler>> elements;
        elements.reserve(max_size);

        std::unique_lock<std::mutex> lk(lock_);
        for (SizeT i = 0; i < queue.size(); ++i) {
            if (queue[i].get() != nullptr) {
                elements.push_back(queue[i]);
            }
        }
        return elements;
    }
};

export struct NewCatalog {
public:
    explicit NewCatalog(std::shared_ptr<std::string> dir, bool create_default_db = false);

public:
    // Database related functions
    std::tuple<DBEntry *, Status> CreateDatabase(const std::string &db_name,
                                                 u64 txn_id,
                                                 TxnTimeStamp begin_ts,
                                                 TxnManager *txn_mgr,
                                                 ConflictType conflict_type = ConflictType::kError);

    std::tuple<DBEntry *, Status> DropDatabase(const std::string &db_name, u64 txn_id, TxnTimeStamp begin_ts, TxnManager *txn_mgr);

    std::tuple<DBEntry *, Status> GetDatabase(const std::string &db_name, u64 txn_id, TxnTimeStamp begin_ts);

    void RemoveDBEntry(const std::string &db_name, u64 txn_id, TxnManager *txn_mgr);

    // List databases
    std::vector<DBEntry *> Databases(u64 txn_id, TxnTimeStamp begin_ts);

    // Table related functions
    std::tuple<TableEntry *, Status> CreateTable(const std::string &db_name,
                                                 u64 txn_id,
                                                 TxnTimeStamp begin_ts,
                                                 const std::shared_ptr<TableDef> &table_def,
                                                 ConflictType conflict_type,
                                                 TxnManager *txn_mgr);

    std::tuple<TableEntry *, Status> DropTableByName(const std::string &db_name,
                                                     const std::string &table_name,
                                                     ConflictType conflict_type,
                                                     u64 txn_id,
                                                     TxnTimeStamp begin_ts,
                                                     TxnManager *txn_mgr);

    Status GetTables(const std::string &db_name, std::vector<TableDetail> &output_table_array, u64 txn_id, TxnTimeStamp begin_ts);

    std::tuple<TableEntry *, Status> GetTableByName(const std::string &db_name, const std::string &table_name, u64 txn_id, TxnTimeStamp begin_ts);

    static Status RemoveTableEntry(TableEntry *table_entry, u64 txn_id, TxnManager *txn_mgr);

    // Index Related methods
    std::tuple<TableEntry *, TableIndexEntry *, Status> CreateIndex(const std::string &db_name,
                                                                    const std::string &table_name,
                                                                    const std::shared_ptr<IndexDef> &index_def,
                                                                    ConflictType conflict_type,
                                                                    u64 txn_id,
                                                                    TxnTimeStamp begin_ts,
                                                                    TxnManager *txn_mgr);

    std::tuple<TableIndexEntry *, Status> DropIndex(const std::string &db_name,
                                                    const std::string &table_name,
                                                    const std::string &index_name,
                                                    ConflictType conflict_type,
                                                    u64 txn_id,
                                                    TxnTimeStamp begin_ts,
                                                    TxnManager *txn_mgr);

    static void CreateIndexFile(TableEntry *table_entry,
                                void *txn_store,
                                TableIndexEntry *table_index_entry,
                                TxnTimeStamp begin_ts,
                                BufferManager *buffer_mgr,
                                bool prepare);

    static Status RemoveIndexEntry(const std::string &index_name, TableIndexEntry *table_index_entry, u64 txn_id, TxnManager *txn_mgr);

    static void CommitCreateIndex(std::unordered_map<std::string, TxnIndexStore> &txn_indexes_store_);

    // Append related functions
    static void Append(TableEntry *table_entry, u64 txn_id, void *txn_store, BufferManager *buffer_mgr);

    static void CommitAppend(TableEntry *table_entry, u64 txn_id, TxnTimeStamp commit_ts, const AppendState *append_state_ptr);

    static void RollbackAppend(TableEntry *table_entry, u64 txn_id, TxnTimeStamp commit_ts, void *txn_store);

    static Status Delete(TableEntry *table_entry, u64 txn_id, TxnTimeStamp commit_ts, DeleteState &delete_state);

    static void CommitDelete(TableEntry *table_entry, u64 txn_id, TxnTimeStamp commit_ts, const DeleteState &append_state);

    static Status RollbackDelete(TableEntry *table_entry, u64 txn_id, DeleteState &append_state, BufferManager *buffer_mgr);

    static Status ImportSegment(TableEntry *table_entry, TxnTimeStamp commit_ts, std::shared_ptr<SegmentEntry> segment);

    static u32 GetNextSegmentID(TableEntry *table_entry);

    static u32 GetMaxSegmentID(const TableEntry *table_entry);

    static void ImportSegment(TableEntry *table_entry, u32 segment_id, std::shared_ptr<SegmentEntry> &segment_entry);

    static void IncreaseTableRowCount(TableEntry *table_entry, u64 increased_row_count);

public:
    // Function related methods
    static std::shared_ptr<FunctionSet> GetFunctionSetByName(NewCatalog *catalog, std::string function_name);

    static void AddFunctionSet(NewCatalog *catalog, const std::shared_ptr<FunctionSet> &function_set);

    static void DeleteFunctionSet(NewCatalog *catalog, std::string function_name);

    // Table Function related methods
    static std::shared_ptr<TableFunction> GetTableFunctionByName(NewCatalog *catalog, std::string function_name);

    static void AddTableFunction(NewCatalog *catalog, const std::shared_ptr<TableFunction> &table_function);

    static void AddSpecialFunction(NewCatalog *catalog, const std::shared_ptr<SpecialFunction> &special_function);

    static std::shared_ptr<SpecialFunction> GetSpecialFunctionByNameNoExcept(NewCatalog *catalog, std::string function_name);

    static void DeleteTableFunction(NewCatalog *catalog, std::string function_name);

public:
    // Serialization and Deserialization
    Json Serialize(TxnTimeStamp max_commit_ts, bool is_full_checkpoint);

    std::string SaveAsFile(const std::string &dir, TxnTimeStamp max_commit_ts, bool is_full_checkpoint);

    void MergeFrom(NewCatalog &other);

    static void Deserialize(const Json &catalog_json, BufferManager *buffer_mgr, std::unique_ptr<NewCatalog> &catalog);

    static std::unique_ptr<NewCatalog> LoadFromFiles(const std::vector<std::string> &catalog_paths, BufferManager *buffer_mgr);

    static std::unique_ptr<NewCatalog> LoadFromFile(const std::string &catalog_path, BufferManager *buffer_mgr);

public:
    // Profile related methods

    void AppendProfilerRecord(std::shared_ptr<QueryProfiler> profiler) { history.Enqueue(std::move(profiler)); }

    const QueryProfiler *GetProfilerRecord(SizeT index) { return history.GetElement(index); }

    const std::vector<std::shared_ptr<QueryProfiler>> GetProfilerRecords() { return history.GetElements(); }

public:
    std::shared_ptr<std::string> current_dir_{nullptr};
    std::unordered_map<std::string, std::unique_ptr<DBMeta>> databases_{};
    u64 next_txn_id_{};
    u64 catalog_version_{};
    std::shared_mutex rw_locker_{};

    // Currently, these function or function set can't be changed and also will not be persistent.
    std::unordered_map<std::string, std::shared_ptr<FunctionSet>> function_sets_{};
    std::unordered_map<std::string, std::shared_ptr<TableFunction>> table_functions_{};
    std::unordered_map<std::string, std::shared_ptr<SpecialFunction>> special_functions_{};

    ProfileHistory history{DEFAULT_PROFILER_HISTORY_SIZE};
};

} // namespace infinity
