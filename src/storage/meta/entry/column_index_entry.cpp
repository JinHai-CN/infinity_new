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

module catalog;

import std;
import index_base;
import third_party;
import local_file_system;
import default_values;
import random;
import buffer_manager;
import infinity_exception;
import index_file_worker;
import parser;
import annivfflat_index_file_worker;
import hnsw_file_worker;
import logger;

namespace infinity {

ColumnIndexEntry::ColumnIndexEntry(std::shared_ptr<IndexBase> index_base,
                                   TableIndexEntry *table_index_entry,
                                   u64 column_id,
                                   std::shared_ptr<std::string> index_dir,
                                   u64 txn_id,
                                   TxnTimeStamp begin_ts)
    : BaseEntry(EntryType::kColumnIndex), table_index_entry_(table_index_entry), column_id_(column_id), index_dir_(index_dir),
      index_base_(index_base) {
    begin_ts_ = begin_ts; // TODO:: begin_ts and txn_id should be const and set in BaseEntry
    txn_id_ = txn_id;
}

std::unique_ptr<ColumnIndexEntry> ColumnIndexEntry::NewColumnIndexEntry(std::shared_ptr<IndexBase> index_base,
                                                                        u64 column_id,
                                                                        TableIndexEntry *table_index_entry,
                                                                        u64 txn_id,
                                                                        std::shared_ptr<std::string> index_dir,
                                                                        TxnTimeStamp begin_ts) {
    //    std::shared_ptr<std::string> index_dir =
    //        DetermineIndexDir(*TableIndexMeta::GetTableEntry(table_index_meta)->table_entry_dir_, index_base->file_name_);
    return std::make_unique<ColumnIndexEntry>(index_base, table_index_entry, column_id, index_dir, txn_id, begin_ts);
}

void ColumnIndexEntry::CommitCreatedIndex(u32 segment_id, std::unique_ptr<SegmentColumnIndexEntry> index_entry) {
    std::unique_lock<std::shared_mutex> w_locker(this->rw_locker_);
    this->index_by_segment_.emplace(segment_id, std::move(index_entry));
}

Json ColumnIndexEntry::Serialize(TxnTimeStamp max_commit_ts) {
    if (this->deleted_) {
        Error<StorageException>("Column index entry can't be deleted.");
    }

    Json json;
    std::vector<SegmentColumnIndexEntry *> segment_column_index_entry_candidates;
    {
        std::shared_lock<std::shared_mutex> lck(this->rw_locker_);

        json["txn_id"] = this->txn_id_.load();
        json["begin_ts"] = this->begin_ts_;
        json["commit_ts"] = this->commit_ts_.load();
        json["deleted"] = this->deleted_;
        json["column_id"] = this->column_id_;
        json["index_dir"] = *this->index_dir();
        json["index_base"] = this->index_base_->Serialize();

        for (const auto &[segment_id, index_entry] : this->index_by_segment_) {
            segment_column_index_entry_candidates.emplace_back((SegmentColumnIndexEntry *)index_entry.get());
        }
    }

    for (const auto &segment_column_index_entry : segment_column_index_entry_candidates) {
        segment_column_index_entry->Flush(max_commit_ts);
        json["index_by_segment"].push_back(segment_column_index_entry->Serialize());
    }

    return json;
}

std::unique_ptr<ColumnIndexEntry> ColumnIndexEntry::Deserialize(const Json &column_index_entry_json,
                                                                TableIndexEntry *table_index_entry,
                                                                BufferManager *buffer_mgr,
                                                                TableEntry *table_entry) {
    bool deleted = column_index_entry_json["deleted"];
    if (deleted) {
        Error<StorageException>("Column index entry can't be deleted.");
    }

    u64 txn_id = column_index_entry_json["txn_id"];
    TxnTimeStamp begin_ts = column_index_entry_json["begin_ts"];
    TxnTimeStamp commit_ts = column_index_entry_json["commit_ts"];
    u64 column_id = column_index_entry_json["column_id"];
    auto index_dir = std::make_shared<std::string>(column_index_entry_json["index_dir"]);
    std::shared_ptr<IndexBase> index_base = IndexBase::Deserialize(column_index_entry_json["index_base"]);

    auto column_index_entry = std::make_unique<ColumnIndexEntry>(index_base, table_index_entry, column_id, index_dir, txn_id, begin_ts);
    column_index_entry->commit_ts_.store(commit_ts);
    column_index_entry->deleted_ = deleted;

    if (column_index_entry_json.contains("index_by_segment")) {
        for (const auto &index_by_segment_json : column_index_entry_json["index_by_segment"]) {
            std::unique_ptr<SegmentColumnIndexEntry> segment_column_index_entry =
                SegmentColumnIndexEntry::Deserialize(index_by_segment_json, column_index_entry.get(), buffer_mgr, table_entry);
            column_index_entry->index_by_segment_.emplace(segment_column_index_entry->segment_id(), std::move(segment_column_index_entry));
        }
    }

    return column_index_entry;
}

std::shared_ptr<std::string> ColumnIndexEntry::DetermineIndexDir(const std::string &parent_dir, const std::string &index_name) {
    LocalFileSystem fs;
    std::shared_ptr<std::string> index_dir;
    do {
        u32 seed = std::time(nullptr);
        index_dir = std::make_shared<std::string>(parent_dir + "/" + RandomString(DEFAULT_RANDOM_NAME_LEN, seed) + "_index_" + index_name);
    } while (!fs.CreateDirectoryNoExp(*index_dir));
    return index_dir;
}

std::unique_ptr<IndexFileWorker> ColumnIndexEntry::CreateFileWorker(CreateIndexParam *param, u32 segment_id) {
    std::unique_ptr<IndexFileWorker> file_worker = nullptr;
    const auto *index_base = param->index_base_;
    const auto *column_def = param->column_def_;
    auto file_name = std::make_shared<std::string>(ColumnIndexEntry::IndexFileName(index_base->file_name_, segment_id));
    switch (index_base->index_type_) {
        case IndexType::kIVFFlat: {
            auto create_annivfflat_param = static_cast<CreateAnnIVFFlatParam *>(param);
            auto elem_type = ((EmbeddingInfo *)(column_def->type()->type_info().get()))->Type();
            switch (elem_type) {
                case kElemFloat: {
                    file_worker = std::make_unique<AnnIVFFlatIndexFileWorker<f32>>(this->index_dir(),
                                                                                   file_name,
                                                                                   index_base,
                                                                                   column_def,
                                                                                   create_annivfflat_param->row_count_);
                    break;
                }
                default: {
                    ExecutorException("Create IVF Flat index: unsupported element type.");
                }
            }
            break;
        }
        case IndexType::kHnsw: {
            auto create_hnsw_param = static_cast<CreateHnswParam *>(param);
            file_worker = std::make_unique<HnswFileWorker>(this->index_dir(), file_name, index_base, column_def, create_hnsw_param->max_element_);
            break;
        }
        case IndexType::kIRSFullText: {
            //            auto create_fulltext_param = static_cast<CreateFullTextParam *>(param);
            std::unique_ptr<std::string> err_msg =
                std::make_unique<std::string>(Format("File worker isn't implemented: {}", IndexInfo::IndexTypeToString(index_base->index_type_)));
            LOG_ERROR(*err_msg);
            Error<StorageException>(*err_msg);
            break;
        }
        default: {
            std::unique_ptr<std::string> err_msg =
                std::make_unique<std::string>(Format("File worker isn't implemented: {}", IndexInfo::IndexTypeToString(index_base->index_type_)));
            LOG_ERROR(*err_msg);
            Error<StorageException>(*err_msg);
        }
    }
    if (file_worker.get() == nullptr) {
        std::unique_ptr<std::string> err_msg = std::make_unique<std::string>("Failed to create index file worker");
        LOG_ERROR(*err_msg);
        Error<StorageException>(*err_msg);
    }
    return file_worker;
}

std::string ColumnIndexEntry::IndexFileName(const std::string &index_name, u32 segment_id) { return Format("seg{}.idx", segment_id, index_name); }

Status ColumnIndexEntry::CreateIndexDo(const ColumnDef *column_def, std::unordered_map<u32, std::atomic<u64>> &create_index_idxes) {
    for (auto &[segment_id, segment_column_index_entry] : index_by_segment_) {
        std::atomic<u64> &create_index_idx = create_index_idxes.at(segment_id);
        auto status = segment_column_index_entry->CreateIndexDo(index_base_.get(), column_def, create_index_idx);
        if (!status.ok()) {
            return status;
        }
    }
    return Status::OK();
}

} // namespace infinity
