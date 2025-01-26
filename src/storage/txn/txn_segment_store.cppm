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

export module txn_segment_store;

import stl;
import data_access_state;
class Txn;

//import status;
//import internal_types;
//import index_base;
//import extra_ddl_info;
//import wal_entry;

struct SegmentEntry;
class CatalogDeltaEntry;
struct BlockColumnEntry;
struct BlockEntry;

namespace infinity {

export struct TxnSegmentStore {
public:
    static TxnSegmentStore AddSegmentStore(SegmentEntry *segment_entry);

    explicit TxnSegmentStore(SegmentEntry *segment_entry);

    TxnSegmentStore() = default;

    void AddDeltaOp(CatalogDeltaEntry *local_delta_ops, AppendState *append_state, Txn *txn, bool set_sealed) const;

public:
    SegmentEntry *const segment_entry_ = nullptr;
    HashMap<BlockID, BlockEntry *> block_entries_;
    Vector<BlockColumnEntry *> block_column_entries_;
};

}
