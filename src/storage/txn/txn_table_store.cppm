// Copyright(C) 2025 InfiniFlow, Inc. All rights reserved.
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

export module txn_table_store;

import stl;
import txn_index_store;
import status;
import txn_segment_store;
import internal_types;
import data_access_state;
import txn_compact_store;
import compact_statement;

class Txn;
class TableEntry;
class SegmentEntry;
class DataBlock;
class TableIndexEntry;
class SegmentIndexEntry;
class ChunkIndexEntry;
class BlockEntry;
class BlockColumnEntry;
class TxnManager;
class CatalogDeltaEntry;
class BufferManager;
struct Catalog;
struct WalSegmentInfo;

namespace infinity {

export class TxnTableStore {
public:
    explicit inline TxnTableStore(Txn *txn, TableEntry *table_entry) : txn_(txn), table_entry_(table_entry) {}

    Tuple<UniquePtr<String>, Status> Import(SharedPtr<SegmentEntry> segment_entry, Txn *txn);

    Tuple<UniquePtr<String>, Status> Append(const SharedPtr<DataBlock> &input_block);

    void AddIndexStore(TableIndexEntry *table_index_entry);

    void AddSegmentIndexesStore(TableIndexEntry *table_index_entry, const Vector<SegmentIndexEntry *> &segment_index_entries);

    void AddChunkIndexStore(TableIndexEntry *table_index_entry, ChunkIndexEntry *chunk_index_entry);

    TxnIndexStore *GetIndexStore(TableIndexEntry *table_index_entry, bool need_lock);

    void DropIndexStore(TableIndexEntry *table_index_entry);

    Tuple<UniquePtr<String>, Status> Delete(const Vector<RowID> &row_ids);

    void SetCompactType(CompactStatementType type);

    Tuple<UniquePtr<String>, Status> Compact(Vector<Pair<SharedPtr<SegmentEntry>, Vector<SegmentEntry *>>> &&segment_data, CompactStatementType type);

    void AddSegmentStore(SegmentEntry *segment_entry);

    void AddBlockStore(SegmentEntry *segment_entry, BlockEntry *block_entry);

    void AddBlockColumnStore(SegmentEntry *segment_entry, BlockEntry *block_entry, BlockColumnEntry *block_column_entry);

    void AddSealedSegment(SegmentEntry *segment_entry);

    void AddDeltaOp(CatalogDeltaEntry *local_delta_ops, TxnManager *txn_mgr, TxnTimeStamp commit_ts, bool added) const;

public:
    // transaction related

    void Rollback(TransactionID txn_id, TxnTimeStamp abort_ts);

    bool CheckConflict(Catalog *catalog, Txn *txn) const;

    Optional<String> CheckConflict(const TxnTableStore *txn_table_store) const;

    void PrepareCommit1(const Vector<WalSegmentInfo *> &segment_infos) const;

    void PrepareCommit(TransactionID txn_id, TxnTimeStamp commit_ts, BufferManager *buffer_mgr);

    void Commit(TransactionID txn_id, TxnTimeStamp commit_ts);

    void MaintainCompactionAlg();

public: // Setter, Getter
    Pair<std::shared_lock<std::shared_mutex>, const HashMap<String, UniquePtr<TxnIndexStore>> &> txn_indexes_store() const;

    const HashMap<SegmentID, TxnSegmentStore> &txn_segments() const { return txn_segments_store_; }

    const Vector<SegmentEntry *> &flushed_segments() const { return flushed_segments_; }

    Txn *GetTxn() const { return txn_; }

    TableEntry *GetTableEntry() const { return table_entry_; }

    inline bool HasUpdate() const { return has_update_; }

    DeleteState &GetDeleteStateRef() { return delete_state_; }

    inline DeleteState *GetDeleteStatePtr() { return &delete_state_; }

    inline const Vector<SharedPtr<DataBlock>> &GetBlocks() const { return blocks_; }

    inline void SetAppendState(UniquePtr<AppendState> append_state) { append_state_ = std::move(append_state); }

    inline AppendState *GetAppendState() const { return append_state_.get(); }

    void AddWriteTxnNum() { added_txn_num_ = true; }

    bool AddedTxnNum() const { return added_txn_num_; }

private:
    mutable std::shared_mutex txn_table_store_mtx_{};

    HashMap<SegmentID, TxnSegmentStore> txn_segments_store_{};
    Vector<SegmentEntry *> flushed_segments_{};
    HashSet<SegmentEntry *> set_sealed_segments_{};

    int ptr_seq_n_;
    HashMap<TableIndexEntry *, int> txn_indexes_{};
    HashMap<String, UniquePtr<TxnIndexStore>> txn_indexes_store_{};

    TxnCompactStore compact_state_;

    Txn *const txn_{};
    Vector<SharedPtr<DataBlock>> blocks_{};

    UniquePtr<AppendState> append_state_{};
    DeleteState delete_state_{};

    SizeT current_block_id_{0};

    TableEntry *table_entry_{};
    bool added_txn_num_{false};

    bool has_update_{false};

public:
    void SetCompacting() { table_status_ = TxnStoreStatus::kCompacting; }

    void SetCreatingIndex() { table_status_ = TxnStoreStatus::kCreatingIndex; }

    void TryRevert();

private:
    enum struct TxnStoreStatus {
        kNone = 0,
        kCreatingIndex,
        kCompacting,
    };
    TxnStoreStatus table_status_{TxnStoreStatus::kNone};
};

} // namespace infinity
