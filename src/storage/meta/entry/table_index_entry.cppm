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

export module catalog:table_index_entry;

import :column_index_entry;
import :segment_column_index_entry;
import :irs_index_entry;
import :base_entry;

import std;
import type_alias;
import index_def;
import third_party;
import status;

namespace infinity {

class TableIndexMeta;
class BufferManager;
struct TableEntry;

struct TableIndexEntry : public BaseEntry {

    friend struct TableEntry;

public:
    TableIndexEntry(const std::shared_ptr<IndexDef> &index_def,
                    TableIndexMeta *table_index_meta,
                    std::shared_ptr<std::string> index_dir,
                    u64 txn_id,
                    TxnTimeStamp begin_ts,
                    bool replay = false);

    TableIndexEntry(TableIndexMeta *table_index_meta, u64 txn_id, TxnTimeStamp begin_ts);

    static std::unique_ptr<TableIndexEntry>
    NewTableIndexEntry(const std::shared_ptr<IndexDef> &index_def, TableIndexMeta *table_index_meta, u64 txn_id, TxnTimeStamp begin_ts);

    static std::unique_ptr<TableIndexEntry> NewDropTableIndexEntry(TableIndexMeta *table_index_meta, u64 txn_id, TxnTimeStamp begin_ts);

    Json Serialize(TxnTimeStamp max_commit_ts);

    static std::unique_ptr<TableIndexEntry>
    Deserialize(const Json &index_def_entry_json, TableIndexMeta *table_index_meta, BufferManager *buffer_mgr, TableEntry *table_entry);

public:
    // Getter
    inline const TableIndexMeta *table_index_meta() const { return table_index_meta_; }
    inline const IndexDef *index_def() const { return index_def_.get(); }
    const std::shared_ptr<IrsIndexEntry> &irs_index_entry() const { return irs_index_entry_; }
    std::unordered_map<u64, std::unique_ptr<ColumnIndexEntry>> &column_index_map() { return column_index_map_; }

    Status CreateIndexDo(const TableEntry *table_entry, std::unordered_map<u32, std::atomic<u64>> &create_index_idxes);

private:
    static std::shared_ptr<std::string> DetermineIndexDir(const std::string &parent_dir, const std::string &index_name);

    void CommitCreateIndex(u64 column_id, u32 segment_id, std::shared_ptr<SegmentColumnIndexEntry> index_entry);
    void CommitCreateIndex(const std::shared_ptr<IrsIndexEntry> &irs_index_entry);

private:
    std::shared_mutex rw_locker_{};
    TableIndexMeta *table_index_meta_{};
    const std::shared_ptr<IndexDef> index_def_{};
    std::shared_ptr<std::string> index_dir_{};

    std::unordered_map<u64, std::unique_ptr<ColumnIndexEntry>> column_index_map_{};

    std::shared_ptr<IrsIndexEntry> irs_index_entry_{};
};

} // namespace infinity
