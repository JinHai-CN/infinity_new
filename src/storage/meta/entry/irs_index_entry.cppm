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

export module catalog:irs_index_entry;

import :base_entry;

import std;
import type_alias;
import index_base;
import third_party;

namespace infinity {

struct TableIndexEntry;
class BufferManager;
struct TableEntry;
class IRSDataStore;
class IndexFullText;

export struct IrsIndexEntry : public BaseEntry {
public:
    IrsIndexEntry(TableIndexEntry *table_index_entry, std::shared_ptr<std::string> index_dir, u64 txn_id, TxnTimeStamp begin_ts);

    static std::shared_ptr<IrsIndexEntry>
    NewIrsIndexEntry(TableIndexEntry *table_index_entry, u64 txn_id, std::shared_ptr<std::string> index_dir, TxnTimeStamp begin_ts);

    void AddColumn(std::shared_ptr<IndexBase> index_base, u64 column_id);

    Json Serialize(TxnTimeStamp max_commit_ts);

    static std::shared_ptr<IrsIndexEntry>
    Deserialize(const Json &index_def_entry_json, TableIndexEntry *table_index_entry, BufferManager *buffer_mgr);

private:
    static std::shared_ptr<std::string> DetermineIndexDir(const std::string &parent_dir, const std::string &index_name);

public:
    std::shared_mutex rw_locker_{};

    const TableIndexEntry *table_index_entry_{};
    std::shared_ptr<std::string> index_dir_{};
    std::unordered_map<u64, std::shared_ptr<IndexFullText>> index_info_map_{};
    std::shared_ptr<IRSDataStore> irs_index_{};
};
} // namespace infinity
