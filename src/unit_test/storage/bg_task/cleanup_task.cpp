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

#include "unit_test/base_test.h"

import stl;
import logger;
import infinity_context;
import storage;
import column_def;
import logical_type;
import data_type;
import txn_manager;
import table_def;
import extra_ddl_info;
import column_vector;
import value;
import catalog;
import segment_entry;
import block_entry;
import buffer_manager;
import physical_import;
import status;
import compilation_config;
import bg_task;
import index_base;
import third_party;
import base_table_ref;
import index_secondary;
import infinity_exception;
import wal_manager;
import compaction_process;

using namespace infinity;

class CleanupTaskTest : public BaseTest {
protected:
    void WaitCleanup(Storage *storage, TxnTimeStamp last_commit_ts) {
        Catalog *catalog = storage->catalog();
        BufferManager *buffer_mgr = storage->buffer_manager();

        auto visible_ts = WaitFlushDeltaOp(storage, last_commit_ts);

        auto cleanup_task = MakeShared<CleanupTask>(catalog, visible_ts, buffer_mgr);
        cleanup_task->Execute();
    }

    TxnTimeStamp WaitFlushDeltaOp(Storage *storage, TxnTimeStamp last_commit_ts) {
        TxnManager *txn_mgr = storage->txn_manager();

        TxnTimeStamp visible_ts = 0;
        time_t start = time(nullptr);
        while (true) {
            visible_ts = txn_mgr->GetCleanupScanTS();
            // wait for at most 10s
            time_t end = time(nullptr);
            if (visible_ts >= last_commit_ts) {
                LOG_INFO(fmt::format("FlushDeltaOp finished after {}", end - start));
                break;
            }
            if (end - start > 10) {
                String error_message = "WaitFlushDeltaOp timeout";
                UnrecoverableError(error_message);
            }
            LOG_INFO(fmt::format("Before usleep. Wait flush delta op for {} seconds", end - start));
            usleep(1000 * 1000);
        }
        return visible_ts;
    }
};

TEST_F(CleanupTaskTest, test_delete_db_simple) {
    // close auto cleanup task
    auto config_path = std::make_shared<std::string>(std::string(test_data_path()) + "/config/test_cleanup_task.toml");

    RemoveDbDirs();
    InfinityContext::instance().Init(config_path);
    Storage *storage = InfinityContext::instance().storage();
    EXPECT_NE(storage, nullptr);

    TxnManager *txn_mgr = storage->txn_manager();
    TxnTimeStamp last_commit_ts = 0;

    auto db_name = MakeShared<String>("db1");
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("create db1"));
        txn->CreateDatabase(*db_name, ConflictType::kError);
        last_commit_ts = txn_mgr->CommitTxn(txn);
    }
    WaitFlushDeltaOp(storage, last_commit_ts);
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("drop db1"));
        Status status = txn->DropDatabase(*db_name, ConflictType::kError);
        EXPECT_TRUE(status.ok());
        last_commit_ts = txn_mgr->CommitTxn(txn);
    }
    WaitCleanup(storage, last_commit_ts);
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("get db1"));
        auto [db_entry, status] = txn->GetDatabase(*db_name);
        EXPECT_EQ(db_entry, nullptr);
        txn_mgr->CommitTxn(txn);
    }
    InfinityContext::instance().UnInit();
}

TEST_F(CleanupTaskTest, test_delete_db_complex) {
    // close auto cleanup task
    auto config_path = std::make_shared<std::string>(std::string(test_data_path()) + "/config/test_cleanup_task.toml");

    RemoveDbDirs();
    InfinityContext::instance().Init(config_path);
    Storage *storage = InfinityContext::instance().storage();
    EXPECT_NE(storage, nullptr);

    TxnManager *txn_mgr = storage->txn_manager();
    TxnTimeStamp last_commit_ts = 0;

    auto db_name = MakeShared<String>("db1");
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("create db1"));
        txn->CreateDatabase(*db_name, ConflictType::kError);
        txn_mgr->CommitTxn(txn);
    }
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("drop db1"));
        Status status = txn->DropDatabase(*db_name, ConflictType::kError);
        EXPECT_TRUE(status.ok());
        txn_mgr->CommitTxn(txn);
    }
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("create db1"));
        txn->CreateDatabase(*db_name, ConflictType::kError);
        txn_mgr->RollBackTxn(txn);
    }
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("create db1"));
        txn->CreateDatabase(*db_name, ConflictType::kError);
        last_commit_ts = txn_mgr->CommitTxn(txn);
    }
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("drop db1"));
        Status status = txn->DropDatabase(*db_name, ConflictType::kError);
        EXPECT_TRUE(status.ok());
        WaitCleanup(storage, last_commit_ts);
        txn_mgr->CommitTxn(txn);
    }
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("get db1"));
        auto [db_entry, status] = txn->GetDatabase(*db_name);
        EXPECT_EQ(db_entry, nullptr);
        txn_mgr->CommitTxn(txn);
    }
    InfinityContext::instance().UnInit();
}

TEST_F(CleanupTaskTest, test_delete_table_simple) {
    // close auto cleanup task
    auto config_path = std::make_shared<std::string>(std::string(test_data_path()) + "/config/test_cleanup_task.toml");

    RemoveDbDirs();
    InfinityContext::instance().Init(config_path);
    Storage *storage = InfinityContext::instance().storage();
    EXPECT_NE(storage, nullptr);

    TxnManager *txn_mgr = storage->txn_manager();
    TxnTimeStamp last_commit_ts = 0;

    Vector<SharedPtr<ColumnDef>> column_defs;
    {
        std::set<ConstraintType> constraints;
        ColumnID column_id = 0;
        column_defs.push_back(MakeShared<ColumnDef>(column_id++, MakeShared<DataType>(DataType(LogicalType::kInteger)), "col1", constraints));
    }
    auto db_name = MakeShared<String>("default_db");
    auto table_name = MakeShared<String>("table1");
    {

        auto table_def = MakeUnique<TableDef>(db_name, table_name, column_defs);
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("create table1"));
        auto status = txn->CreateTable(*db_name, std::move(table_def), ConflictType::kIgnore);
        EXPECT_TRUE(status.ok());
        last_commit_ts = txn_mgr->CommitTxn(txn);
    }
    WaitFlushDeltaOp(storage, last_commit_ts);
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("drop table1"));

        Status status = txn->DropTableCollectionByName(*db_name, *table_name, ConflictType::kError);
        EXPECT_TRUE(status.ok());

        last_commit_ts = txn_mgr->CommitTxn(txn);
    }

    WaitCleanup(storage, last_commit_ts);
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("get table1"));
        auto [table_entry, status] = txn->GetTableByName(*db_name, *table_name);
        EXPECT_EQ(table_entry, nullptr);

        txn_mgr->CommitTxn(txn);
    }

    InfinityContext::instance().UnInit();
}

TEST_F(CleanupTaskTest, test_delete_table_complex) {
    // close auto cleanup task
    auto config_path = std::make_shared<std::string>(std::string(test_data_path()) + "/config/test_cleanup_task.toml");

    RemoveDbDirs();
    InfinityContext::instance().Init(config_path);
    Storage *storage = InfinityContext::instance().storage();
    EXPECT_NE(storage, nullptr);

    TxnManager *txn_mgr = storage->txn_manager();
    TxnTimeStamp last_commit_ts = 0;

    Vector<SharedPtr<ColumnDef>> column_defs;
    {
        std::set<ConstraintType> constraints;
        ColumnID column_id = 0;
        column_defs.push_back(MakeShared<ColumnDef>(column_id++, MakeShared<DataType>(DataType(LogicalType::kInteger)), "col1", constraints));
    }
    auto db_name = MakeShared<String>("default_db");
    auto table_name = MakeShared<String>("table1");
    {

        auto table_def = MakeUnique<TableDef>(db_name, table_name, column_defs);
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("create table1"));
        auto status = txn->CreateTable(*db_name, std::move(table_def), ConflictType::kIgnore);
        EXPECT_TRUE(status.ok());
        txn_mgr->CommitTxn(txn);
    }
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("drop table1"));

        Status status = txn->DropTableCollectionByName(*db_name, *table_name, ConflictType::kError);
        EXPECT_TRUE(status.ok());

        txn_mgr->CommitTxn(txn);
    }
    {
        auto table_def = MakeUnique<TableDef>(db_name, table_name, column_defs);
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("create table1"));
        auto status = txn->CreateTable(*db_name, std::move(table_def), ConflictType::kIgnore);
        EXPECT_TRUE(status.ok());
        txn_mgr->RollBackTxn(txn);
    }
    {
        auto table_def = MakeUnique<TableDef>(db_name, table_name, column_defs);
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("create table1"));
        auto status = txn->CreateTable(*db_name, std::move(table_def), ConflictType::kIgnore);
        EXPECT_TRUE(status.ok());
        last_commit_ts = txn_mgr->CommitTxn(txn);
    }

    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("drop table1"));

        Status status = txn->DropTableCollectionByName(*db_name, *table_name, ConflictType::kError);
        EXPECT_TRUE(status.ok());
        WaitCleanup(storage, last_commit_ts);
        txn_mgr->CommitTxn(txn);
    }
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("get table1"));
        auto [table_entry, status] = txn->GetTableByName(*db_name, *table_name);
        EXPECT_EQ(table_entry, nullptr);

        txn_mgr->CommitTxn(txn);
    }

    InfinityContext::instance().UnInit();
}

TEST_F(CleanupTaskTest, test_compact_and_cleanup) {
    constexpr int kImportN = 5;
    constexpr int kImportSize = 100;

    // disable auto cleanup task
    auto config_path = std::make_shared<std::string>(std::string(test_data_path()) + "/config/test_cleanup_task.toml");

    RemoveDbDirs();
    InfinityContext::instance().Init(config_path);
    Storage *storage = InfinityContext::instance().storage();
    EXPECT_NE(storage, nullptr);

    TxnManager *txn_mgr = storage->txn_manager();
    BufferManager *buffer_mgr = storage->buffer_manager();
    CompactionProcessor *compaction_processor = storage->compaction_processor();
    TxnTimeStamp last_commit_ts = 0;

    Vector<SharedPtr<ColumnDef>> column_defs;
    {
        std::set<ConstraintType> constraints;
        ColumnID column_id = 0;
        column_defs.push_back(MakeShared<ColumnDef>(column_id++, MakeShared<DataType>(DataType(LogicalType::kInteger)), "col1", constraints));
    }
    auto db_name = MakeShared<String>("default_db");
    auto table_name = MakeShared<String>("table1");
    {

        auto table_def = MakeUnique<TableDef>(db_name, table_name, column_defs);
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("create table1"));
        auto status = txn->CreateTable(*db_name, std::move(table_def), ConflictType::kIgnore);
        EXPECT_TRUE(status.ok());
        txn_mgr->CommitTxn(txn);
    }
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("get table1"));
        auto [table_entry, status] = txn->GetTableByName(*db_name, *table_name);
        EXPECT_TRUE(table_entry != nullptr);
        // table_entry->SetCompactionAlg(nullptr);

        for (int i = 0; i < kImportN; ++i) {
            Vector<SharedPtr<ColumnVector>> column_vectors;
            {
                SharedPtr<ColumnVector> column_vector = ColumnVector::Make(MakeShared<DataType>(column_defs[0]->type()->type()));
                column_vector->Initialize();
                Value v = Value::MakeInt(i);
                for (int i = 0; i < kImportSize; ++i) {
                    column_vector->AppendValue(v);
                }
                column_vectors.push_back(column_vector);
            }
            SizeT column_count = column_vectors.size();

            SegmentID segment_id = Catalog::GetNextSegmentID(table_entry);
            auto segment_entry = SegmentEntry::NewSegmentEntry(table_entry, segment_id, txn);
            auto block_entry = BlockEntry::NewBlockEntry(segment_entry.get(), 0, 0, column_count, txn);
            SizeT row_count = std::numeric_limits<SizeT>::max();
            for (SizeT i = 0; i < column_count; ++i) {
                auto *column_vector = column_vectors[i].get();
                auto column_block_entry = block_entry->GetColumnBlockEntry(i);
                if (row_count == std::numeric_limits<SizeT>::max()) {
                    row_count = column_vector->Size();
                } else {
                    EXPECT_EQ(row_count, column_vector->Size());
                }
                column_block_entry->Append(column_vector, 0, row_count, buffer_mgr);
            }
            block_entry->IncreaseRowCount(row_count);
            segment_entry->AppendBlockEntry(std::move(block_entry));

            PhysicalImport::SaveSegmentData(table_entry, txn, segment_entry);
        }
        txn_mgr->CommitTxn(txn);
    }
    {
        auto last_commit_ts = compaction_processor->ManualDoCompact(*db_name, *table_name, false);
        EXPECT_NE(last_commit_ts, 0u);
    }
    WaitCleanup(storage, last_commit_ts);

    InfinityContext::instance().UnInit();
}

TEST_F(CleanupTaskTest, test_with_index_compact_and_cleanup) {
    constexpr int kImportN = 5;
    constexpr int kImportSize = 100;

    // close auto cleanup task
    auto config_path = std::make_shared<std::string>(std::string(test_data_path()) + "/config/test_cleanup_task.toml");

    RemoveDbDirs();
    InfinityContext::instance().Init(config_path);
    Storage *storage = InfinityContext::instance().storage();
    EXPECT_NE(storage, nullptr);

    TxnManager *txn_mgr = storage->txn_manager();
    BufferManager *buffer_mgr = storage->buffer_manager();
    CompactionProcessor *compaction_processor = storage->compaction_processor();
    TxnTimeStamp last_commit_ts = 0;

    auto db_name = MakeShared<String>("default_db");
    auto table_name = MakeShared<String>("table1");
    auto index_name = MakeShared<String>("idx1");
    auto column_name = MakeShared<String>("col1");

    Vector<SharedPtr<ColumnDef>> column_defs;
    {
        std::set<ConstraintType> constraints;
        ColumnID column_id = 0;
        column_defs.push_back(MakeShared<ColumnDef>(column_id++, MakeShared<DataType>(DataType(LogicalType::kInteger)), *column_name, constraints));
    }
    {
        auto table_def = MakeUnique<TableDef>(db_name, table_name, column_defs);
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("create table1"));
        auto status = txn->CreateTable(*db_name, std::move(table_def), ConflictType::kIgnore);
        EXPECT_TRUE(status.ok());
        txn_mgr->CommitTxn(txn);
    }
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("get table1"));
        auto [table_entry, status] = txn->GetTableByName(*db_name, *table_name);
        EXPECT_TRUE(table_entry != nullptr);
        // table_entry->SetCompactionAlg(nullptr);

        for (int i = 0; i < kImportN; ++i) {
            Vector<SharedPtr<ColumnVector>> column_vectors;
            {
                SharedPtr<ColumnVector> column_vector = ColumnVector::Make(MakeShared<DataType>(column_defs[0]->type()->type()));
                column_vector->Initialize();
                Value v = Value::MakeInt(i);
                for (int i = 0; i < kImportSize; ++i) {
                    column_vector->AppendValue(v);
                }
                column_vectors.push_back(column_vector);
            }
            SizeT column_count = column_vectors.size();

            SegmentID segment_id = Catalog::GetNextSegmentID(table_entry);
            auto segment_entry = SegmentEntry::NewSegmentEntry(table_entry, segment_id, txn);
            auto block_entry = BlockEntry::NewBlockEntry(segment_entry.get(), 0, 0, column_count, txn);
            SizeT row_count = std::numeric_limits<SizeT>::max();
            for (SizeT i = 0; i < column_count; ++i) {
                auto *column_vector = column_vectors[i].get();
                auto column_block_entry = block_entry->GetColumnBlockEntry(i);
                if (row_count == std::numeric_limits<SizeT>::max()) {
                    row_count = column_vector->Size();
                } else {
                    EXPECT_EQ(row_count, column_vector->Size());
                }
                column_block_entry->Append(column_vector, 0, row_count, buffer_mgr);
            }
            block_entry->IncreaseRowCount(row_count);
            segment_entry->AppendBlockEntry(std::move(block_entry));

            PhysicalImport::SaveSegmentData(table_entry, txn, segment_entry);
        }
        txn_mgr->CommitTxn(txn);
    }
    {
        auto *txn = txn_mgr->BeginTxn(MakeUnique<String>("create index"));

        SharedPtr<IndexBase> index_base = IndexSecondary::Make(index_name, fmt::format("{}_{}", *table_name, *index_name), {*column_name});

        auto [table_entry, status1] = txn->GetTableByName(*db_name, *table_name);
        EXPECT_TRUE(status1.ok());

        auto table_ref = BaseTableRef::FakeTableRef(table_entry, txn);
        auto [table_index_entry, status2] = txn->CreateIndexDef(table_entry, index_base, ConflictType::kError);
        EXPECT_TRUE(status2.ok());

        auto [_, status3] = txn->CreateIndexPrepare(table_index_entry, table_ref.get(), false);
        txn->CreateIndexFinish(table_entry, table_index_entry);
        EXPECT_TRUE(status3.ok());

        txn_mgr->CommitTxn(txn);
    }
    {
        last_commit_ts = compaction_processor->ManualDoCompact(*db_name, *table_name, false);
        EXPECT_NE(last_commit_ts, 0u);
    }

    WaitCleanup(storage, last_commit_ts);

    InfinityContext::instance().UnInit();
}