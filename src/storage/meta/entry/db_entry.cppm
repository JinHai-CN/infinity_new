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

export module catalog:db_entry;

import :table_meta;
import :base_entry;

import std;
import type_alias;
import table_entry_type;
import parser;
import third_party;
import table_detail;
import buffer_manager;
import status;

namespace infinity {

class DBEntry : public BaseEntry {
    friend struct NewCatalog;

public:
    inline explicit DBEntry(const std::shared_ptr<std::string> &data_dir, std::shared_ptr<std::string> db_name, u64 txn_id, TxnTimeStamp begin_ts)
        : BaseEntry(EntryType::kDatabase), db_entry_dir_(std::make_shared<std::string>(Format("{}/{}/txn_{}", *data_dir, *db_name, txn_id))),
          db_name_(std::move(db_name)) {
        begin_ts_ = begin_ts;
        txn_id_ = txn_id;
    }

public:
    std::shared_ptr<std::string> ToString();

    Json Serialize(TxnTimeStamp max_commit_ts, bool is_full_checkpoint);

    static std::unique_ptr<DBEntry> Deserialize(const Json &db_entry_json, BufferManager *buffer_mgr);

    virtual void MergeFrom(BaseEntry &other);

    [[nodiscard]] const std::string &db_name() const { return *db_name_; }

    [[nodiscard]] const std::shared_ptr<std::string> &db_name_ptr() const { return db_name_; }

private:
    std::tuple<TableEntry *, Status> CreateTable(TableEntryType table_entry_type,
                                                 const std::shared_ptr<std::string> &table_collection_name,
                                                 const std::vector<std::shared_ptr<ColumnDef>> &columns,
                                                 u64 txn_id,
                                                 TxnTimeStamp begin_ts,
                                                 TxnManager *txn_mgr);

    std::tuple<TableEntry *, Status>
    DropTable(const std::string &table_collection_name, ConflictType conflict_type, u64 txn_id, TxnTimeStamp begin_ts, TxnManager *txn_mgr);

    std::tuple<TableEntry *, Status> GetTableCollection(const std::string &table_name, u64 txn_id, TxnTimeStamp begin_ts);

    void RemoveTableEntry(const std::string &table_collection_name, u64 txn_id, TxnManager *txn_mgr);

    std::vector<TableEntry *> TableCollections(u64 txn_id, TxnTimeStamp begin_ts);

    Status GetTablesDetail(u64 txn_id, TxnTimeStamp begin_ts, std::vector<TableDetail> &output_table_array);

private:
    std::shared_mutex rw_locker_{};
    std::shared_ptr<std::string> db_entry_dir_{};
    std::shared_ptr<std::string> db_name_{};
    std::unordered_map<std::string, std::unique_ptr<TableMeta>> tables_{}; // NOTE : can use std::shared_ptr<std::string> as key.
};

} // namespace infinity
