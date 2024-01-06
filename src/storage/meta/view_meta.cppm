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

export module catalog:view_meta;

import :base_entry;

import std;
import type_alias;
import parser;
import status;

namespace infinity {

class TxnManager;

class DBEntry;

export struct ViewMeta {
public:
    explicit ViewMeta(std::shared_ptr<std::string> name, DBEntry *db_entry) : view_name_(std::move(name)), db_entry_(db_entry) {}

public:
    static Status CreateNewEntry(ViewMeta *table_meta,
                                 const std::shared_ptr<std::string> &view_name,
                                 const std::vector<std::shared_ptr<ColumnDef>> &columns,
                                 u64 txn_id,
                                 TxnTimeStamp begin_ts,
                                 TxnManager *txn_mgr);

    static std::shared_ptr<std::string> ToString(ViewMeta *table_meta);

private:
    Status DropNewEntry(u64 txn_id, TxnTimeStamp begin_ts, TxnManager *txn_mgr, const std::string &table_name);

    void DeleteNewEntry(u64 txn_id, TxnManager *txn_mgr);

    Status GetEntry(u64 txn_id, TxnTimeStamp begin_ts);

    inline void *GetDBEntry() { return this->db_entry_; }

private:
    std::shared_mutex rw_locker_{};
    std::shared_ptr<std::string> view_name_{};

    DBEntry *db_entry_{};

    // Ordered by commit_ts from latest to oldest.
    std::list<std::unique_ptr<BaseEntry>> entry_list_{};
};

} // namespace infinity
