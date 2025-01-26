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

export module txn_index_store;

import stl;

namespace infinity {

struct TableIndexEntry;
class SegmentIndexEntry;
struct ChunkIndexEntry;
class CatalogDeltaEntry;

export struct TxnIndexStore {
public:
    explicit TxnIndexStore(TableIndexEntry *table_index_entry);
    TxnIndexStore() = default;

    void AddDeltaOp(CatalogDeltaEntry *local_delta_ops, TxnTimeStamp commit_ts) const;

    void Commit(TransactionID txn_id, TxnTimeStamp commit_ts);

    void Rollback(TxnTimeStamp abort_ts);

    void AddSegmentOptimizing(SegmentIndexEntry *segment_index_entry);

    bool TryRevert();

public:
    TableIndexEntry *const table_index_entry_{};

    HashMap<SegmentID, SegmentIndexEntry *> index_entry_map_{};
    HashMap<String, ChunkIndexEntry *> chunk_index_entries_{};

    Vector<Tuple<SegmentIndexEntry *, ChunkIndexEntry *, Vector<ChunkIndexEntry *>>> optimize_data_;

    enum struct TxnStoreStatus {
        kNone,
        kOptimizing,
    };
    TxnStoreStatus status_{TxnStoreStatus::kNone};
};

} // namespace infinity
