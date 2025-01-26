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

export module txn_store;

import stl;
import data_access_state;
import status;
import internal_types;
import index_base;
import extra_ddl_info;
import wal_entry;
import txn_table_store;

namespace infinity {

class Txn;
struct Catalog;
struct DBEntry;
struct TableIndexEntry;
struct TableEntry;

class DataBlock;
struct SegmentIndexEntry;
struct ChunkIndexEntry;
class BGTaskProcessor;
class TxnManager;
class CatalogDeltaEntry;
class BufferManager;


export class TxnStore {
public:
    explicit TxnStore(Txn *txn);

    void AddDBStore(DBEntry *db_entry);

    void DropDBStore(DBEntry *dropped_db_entry);

    void AddTableStore(TableEntry *table_entry);

    void DropTableStore(TableEntry *dropped_table_entry);

    TxnTableStore *GetTxnTableStore(TableEntry *table_entry);

    TxnTableStore *GetExistTxnTableStore(TableEntry *table_entry) const;

    void AddDeltaOp(CatalogDeltaEntry *local_delta_opsm, TxnManager *txn_mgr) const;

    void MaintainCompactionAlg() const;

    bool CheckConflict(Catalog *catalog);

    Optional<String> CheckConflict(const TxnStore &txn_store);

    void PrepareCommit1();

    void PrepareCommit(TransactionID txn_id, TxnTimeStamp commit_ts, BufferManager *buffer_mgr);

    void CommitBottom(TransactionID txn_id, TxnTimeStamp commit_ts);

    void Rollback(TransactionID txn_id, TxnTimeStamp abort_ts);

    bool ReadOnly() const;

    std::mutex mtx_{};

    void RevertTableStatus();

    void SetCompacting(TableEntry *table_entry);

    void SetCreatingIndex(TableEntry *table_entry);

    void AddSemaphore(UniquePtr<std::binary_semaphore> sema) { semas_.push_back(std::move(sema)); }

    const Vector<UniquePtr<std::binary_semaphore>> &semas() const { return semas_; }

private:
    // Txn store
    Txn *txn_{}; // TODO: remove this
    int ptr_seq_n_{};
    HashMap<DBEntry *, int> txn_dbs_{};
    HashMap<TableEntry *, int> txn_tables_{};
    // Key: table name Value: TxnTableStore
    HashMap<String, UniquePtr<TxnTableStore>> txn_tables_store_{};

    Vector<UniquePtr<std::binary_semaphore>> semas_{};
};

} // namespace infinity
