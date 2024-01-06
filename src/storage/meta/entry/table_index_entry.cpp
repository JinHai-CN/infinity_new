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

// #include <ctime>
// #include <memory>

module catalog;

import std;
import type_alias;
import index_def;
import third_party;
import local_file_system;
import default_values;
import random;
import index_base;
import parser;
import infinity_exception;
import index_full_text;

namespace infinity {

TableIndexEntry::TableIndexEntry(const std::shared_ptr<IndexDef> &index_def,
                                 TableIndexMeta *table_index_meta,
                                 std::shared_ptr<std::string> index_dir,
                                 u64 txn_id,
                                 TxnTimeStamp begin_ts,
                                 bool is_replay)
    : BaseEntry(EntryType::kTableIndex), table_index_meta_(table_index_meta), index_def_(std::move(index_def)), index_dir_(std::move(index_dir)) {
    begin_ts_ = begin_ts; // TODO:: begin_ts and txn_id should be const and set in BaseEntry
    txn_id_ = txn_id;

    SizeT index_count = index_def->index_array_.size();
    column_index_map_.reserve(index_count);
    if (is_replay) {
        return;
    }
    std::unordered_map<u64, std::shared_ptr<IndexFullText>> index_info_map;
    for (SizeT idx = 0; idx < index_count; ++idx) {
        std::shared_ptr<IndexBase> &index_base = index_def->index_array_[idx];

        // Get column info
        if (index_base->column_names_.size() != 1) {
            Error<StorageException>("Currently, composite index doesn't supported.");
        }
        u64 column_id = table_index_meta->GetTableEntry()->GetColumnIdByName(index_base->column_names_[0]);
        if (index_base->index_type_ == IndexType::kIRSFullText) {
            index_info_map.emplace(column_id, std::static_pointer_cast<IndexFullText>(index_base));
        } else {
            std::shared_ptr<std::string> column_index_path =
                std::make_shared<std::string>(Format("{}/{}", *index_dir_, index_base->column_names_[0]));
            std::unique_ptr<ColumnIndexEntry> column_index_entry =
                ColumnIndexEntry::NewColumnIndexEntry(index_base, column_id, this, txn_id, column_index_path, begin_ts);
            column_index_map_[column_id] = std::move(column_index_entry);
        }
    }
    if (!index_info_map.empty()) {
        irs_index_entry_ = IrsIndexEntry::NewIrsIndexEntry(this, txn_id, index_dir_, begin_ts);
    }
}

TableIndexEntry::TableIndexEntry(TableIndexMeta *table_index_meta, u64 txn_id, TxnTimeStamp begin_ts)
    : BaseEntry(EntryType::kTableIndex), table_index_meta_(table_index_meta) {
    begin_ts_ = begin_ts; // TODO:: begin_ts and txn_id should be const and set in BaseEntry
    txn_id_ = txn_id;
}

std::unique_ptr<TableIndexEntry>
TableIndexEntry::NewTableIndexEntry(const std::shared_ptr<IndexDef> &index_def, TableIndexMeta *table_index_meta, u64 txn_id, TxnTimeStamp begin_ts) {
    std::shared_ptr<std::string> index_dir = DetermineIndexDir(*table_index_meta->GetTableEntry()->TableEntryDir(), *index_def->index_name_);
    return std::make_unique<TableIndexEntry>(index_def, table_index_meta, index_dir, txn_id, begin_ts);
}

std::unique_ptr<TableIndexEntry> TableIndexEntry::NewDropTableIndexEntry(TableIndexMeta *table_index_meta, u64 txn_id, TxnTimeStamp begin_ts) {
    return std::make_unique<TableIndexEntry>(table_index_meta, txn_id, begin_ts);
}

void TableIndexEntry::CommitCreateIndex(u64 column_id, u32 segment_id, std::shared_ptr<SegmentColumnIndexEntry> segment_column_index_entry) {
    std::unique_lock<std::shared_mutex> w_locker(this->rw_locker_);
    ColumnIndexEntry *column_index_entry = this->column_index_map_[column_id].get();
    column_index_entry->index_by_segment_.emplace(segment_id, segment_column_index_entry);
}

void TableIndexEntry::CommitCreateIndex(const std::shared_ptr<IrsIndexEntry> &irs_index_entry) { this->irs_index_entry_ = irs_index_entry; }

Json TableIndexEntry::Serialize(TxnTimeStamp max_commit_ts) {
    Json json;

    std::vector<ColumnIndexEntry *> column_index_entry_candidates;
    IrsIndexEntry *irs_index_entry_candidate_{};
    {
        std::shared_lock<std::shared_mutex> lck(this->rw_locker_);
        json["txn_id"] = this->txn_id_.load();
        json["begin_ts"] = this->begin_ts_;
        json["commit_ts"] = this->commit_ts_.load();
        json["deleted"] = this->deleted_;
        if (this->deleted_) {
            return json;
        }

        json["index_dir"] = *this->index_dir_;
        json["index_def"] = this->index_def_->Serialize();

        for (const auto &[index_name, column_index_entry] : this->column_index_map_) {
            column_index_entry_candidates.emplace_back((ColumnIndexEntry *)column_index_entry.get());
        }

        irs_index_entry_candidate_ = this->irs_index_entry_.get();
    }

    for (const auto &column_index_entry : column_index_entry_candidates) {
        json["column_indexes"].emplace_back(column_index_entry->Serialize(max_commit_ts));
    }

    if (irs_index_entry_candidate_ != nullptr) {
        json["irs_index_entry"] = irs_index_entry_candidate_->Serialize(max_commit_ts);
    }

    return json;
}

std::unique_ptr<TableIndexEntry>
TableIndexEntry::Deserialize(const Json &index_def_entry_json, TableIndexMeta *table_index_meta, BufferManager *buffer_mgr, TableEntry *table_entry) {
    u64 txn_id = index_def_entry_json["txn_id"];
    TxnTimeStamp begin_ts = index_def_entry_json["begin_ts"];
    TxnTimeStamp commit_ts = index_def_entry_json["commit_ts"];
    bool deleted = index_def_entry_json["deleted"];

    if (deleted) {
        auto table_index_entry = NewDropTableIndexEntry(table_index_meta, txn_id, begin_ts);
        table_index_entry->deleted_ = true;
        table_index_entry->commit_ts_.store(commit_ts);
        table_index_entry->begin_ts_ = begin_ts;
        return table_index_entry;
    }

    auto index_dir = std::make_shared<std::string>(index_def_entry_json["index_dir"]);
    auto index_def = IndexDef::Deserialize(index_def_entry_json["index_def"]);

    std::unique_ptr<TableIndexEntry> table_index_entry =
        std::make_unique<TableIndexEntry>(index_def, table_index_meta, index_dir, txn_id, begin_ts, true);
    table_index_entry->commit_ts_.store(commit_ts);
    table_index_entry->begin_ts_ = begin_ts;

    if (index_def_entry_json.contains("column_indexes")) {
        for (const auto &column_index_entry_json : index_def_entry_json["column_indexes"]) {
            std::unique_ptr<ColumnIndexEntry> column_index_entry =
                ColumnIndexEntry::Deserialize(column_index_entry_json, table_index_entry.get(), buffer_mgr, table_entry);
            u64 column_id = column_index_entry->column_id_;
            table_index_entry->column_index_map_.emplace(column_id, std::move(column_index_entry));
        }
    }

    if (index_def_entry_json.contains("irs_index_entry")) {
        table_index_entry->irs_index_entry_ =
            IrsIndexEntry::Deserialize(index_def_entry_json["irs_index_entry"], table_index_entry.get(), buffer_mgr);
    }
    return table_index_entry;
}

std::shared_ptr<std::string> TableIndexEntry::DetermineIndexDir(const std::string &parent_dir, const std::string &index_name) {
    LocalFileSystem fs;
    std::shared_ptr<std::string> index_dir;
    do {
        u32 seed = std::time(nullptr);
        index_dir = std::make_shared<std::string>(Format("{}/{}_index_{}", parent_dir, RandomString(DEFAULT_RANDOM_NAME_LEN, seed), index_name));
    } while (!fs.CreateDirectoryNoExp(*index_dir));
    return index_dir;
}

Status TableIndexEntry::CreateIndexDo(const TableEntry *table_entry, std::unordered_map<u32, std::atomic<u64>> &create_index_idxes) {
    if (column_index_map_.size() != 1) {
        // TODO
        Error<NotImplementException>("Not implemented");
    }
    const auto &[column_id, column_index_entry] = *column_index_map_.begin();

    const auto *column_def = table_entry->GetColumnDefByID(column_id);
    return column_index_entry->CreateIndexDo(column_def, create_index_idxes);
}

} // namespace infinity
