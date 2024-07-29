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

#include <functional>
#include <memory>

module txn_manager;

import txn;
import txn_state;
import stl;
import third_party;

import infinity_exception;
import logger;
import buffer_manager;
import background_process;
import catalog_delta_entry;
import default_values;
import wal_manager;
import bg_task;

namespace infinity {

TxnManager::TxnManager(Catalog *catalog,
                       BufferManager *buffer_mgr,
                       BGTaskProcessor *bg_task_processor,
                       WalManager *wal_mgr,
                       TransactionID start_txn_id,
                       TxnTimeStamp start_ts,
                       bool enable_compaction)
    : catalog_(catalog), buffer_mgr_(buffer_mgr), bg_task_processor_(bg_task_processor), wal_mgr_(wal_mgr), start_ts_(start_ts), is_running_(false),
      enable_compaction_(enable_compaction) {}

Txn *TxnManager::BeginTxn(UniquePtr<String> txn_text, bool ckp_txn) {
    // Check if the is_running_ is true
    if (is_running_.load() == false) {
        String error_message = "TxnManager is not running, cannot create txn";
        UnrecoverableError(error_message);
    }

    std::lock_guard guard(locker_);

    // Assign a new txn id
    u64 new_txn_id = ++catalog_->next_txn_id_;

    // Record the start ts of the txn
    TxnTimeStamp begin_ts = ++start_ts_;
    if (ckp_txn) {
        if (ckp_begin_ts_ != UNCOMMIT_TS) {
            // not set ckp_begin_ts_ may not truncate the wal file.
            LOG_WARN(fmt::format("Another checkpoint txn is started in {}, new checkpoint {} will do nothing", ckp_begin_ts_, begin_ts));
        } else {
            LOG_DEBUG(fmt::format("Checkpoint txn is started in {}", begin_ts));
            ckp_begin_ts_ = begin_ts;
        }
    }

    // Create txn instance
    auto new_txn = SharedPtr<Txn>(new Txn(this, buffer_mgr_, catalog_, bg_task_processor_, new_txn_id, begin_ts, std::move(txn_text)));

    // Storage txn in txn manager
    txn_map_[new_txn_id] = new_txn;
    begin_txn_by_ts_.emplace(begin_ts, new_txn_id);
//    beginned_txns_.emplace_back(new_txn);

    // LOG_INFO(fmt::format("Txn: {} is Begin. begin ts: {}", new_txn_id, begin_ts));
    return new_txn.get();
}

Txn *TxnManager::GetTxn(TransactionID txn_id) const {
    std::lock_guard guard(locker_);
    Txn *res = txn_map_.at(txn_id).get();
    return res;
}

TxnState TxnManager::GetTxnState(TransactionID txn_id) const {
    std::lock_guard guard(locker_);
    auto iter = txn_map_.find(txn_id);
    if (iter == txn_map_.end()) {
        return TxnState::kCommitted;
    }
    Txn *txn = iter->second.get();
    return txn->GetTxnState();
}

bool TxnManager::CheckIfCommitting(TransactionID txn_id, TxnTimeStamp begin_ts) {
    std::lock_guard guard(locker_);
    auto iter = txn_map_.find(txn_id);
    if (iter == txn_map_.end()) {
        return true; // Txn is already committed
    }
    Txn *txn = iter->second.get();
    auto state = txn->GetTxnState();
    if (state != TxnState::kCommitting && state != TxnState::kCommitted) {
        return false;
    }
    return txn->CommitTS() < begin_ts;
}

TxnTimeStamp TxnManager::GetCommitTimeStampW(Txn *txn) {
    txn->SetTxnWrite();

    std::lock_guard guard(locker_);
    TxnTimeStamp commit_ts = ++start_ts_;
    wait_conflict_ck_.emplace(commit_ts, nullptr);
    finishing_txns_.emplace(txn);
    return commit_ts;
}

bool TxnManager::CheckConflict(Txn *txn) {
    TxnTimeStamp begin_ts = txn->BeginTS();
    TxnTimeStamp commit_ts = txn->CommitTS();
    Vector<Txn *> candidate_txns;
    {
        std::lock_guard guard(locker_);
        // LOG_INFO(fmt::format("Txn {} check conflict", txn->TxnID()));
        {
            auto iter = std::lower_bound(finished_txns_.begin(), finished_txns_.end(), begin_ts, [](Txn *txn, TxnTimeStamp begin_ts) {
                return txn->CommittedTS() < begin_ts;
            });
            candidate_txns.insert(candidate_txns.end(), iter, finished_txns_.end());
        }
        for (auto *finishing_txn : finishing_txns_) {
            TxnTimeStamp finishing_commit_ts = finishing_txn->CommitTS();
            if (commit_ts > finishing_commit_ts) {
                candidate_txns.push_back(finishing_txn);
            }
        }
    }
    for (auto *candidate_txn : candidate_txns) {
        // LOG_INFO(fmt::format("Txn {} check conflict with txn {}", txn->TxnID(), candidate_txn->TxnID()));
        if (txn->CheckConflict(candidate_txn)) {
            return true;
        }
    }
    return false;
}

void TxnManager::SendToWAL(Txn *txn) {
    // Check if the is_running_ is true
    if (is_running_.load() == false) {
        String error_message = "TxnManager is not running, cannot put wal entry";
        UnrecoverableError(error_message);
    }
    if (wal_mgr_ == nullptr) {
        String error_message = "TxnManager is null";
        UnrecoverableError(error_message);
    }

    TxnTimeStamp commit_ts = txn->CommitTS();
    WalEntry *wal_entry = txn->GetWALEntry();

    std::lock_guard guard(locker_);
    if (wait_conflict_ck_.empty()) {
        String error_message = fmt::format("WalManager::PutEntry wait_conflict_ck_ is empty, txn->CommitTS() {}", txn->CommitTS());
        UnrecoverableError(error_message);
    }
    if (wait_conflict_ck_.begin()->first > commit_ts) {
        String error_message = fmt::format("WalManager::PutEntry wait_conflict_ck_.begin()->first {} > txn->CommitTS() {}",
                                           wait_conflict_ck_.begin()->first,
                                           txn->CommitTS());
        UnrecoverableError(error_message);
    }
    if (wal_entry) {
        wait_conflict_ck_.at(commit_ts) = wal_entry;
    } else {
        wait_conflict_ck_.erase(commit_ts); // rollback
    }
    if (!wait_conflict_ck_.empty() && wait_conflict_ck_.begin()->second != nullptr) {
        Vector<WalEntry *> wal_entries;
        do {
            wal_entries.push_back(wait_conflict_ck_.begin()->second);
            wait_conflict_ck_.erase(wait_conflict_ck_.begin());
        } while (!wait_conflict_ck_.empty() && wait_conflict_ck_.begin()->second != nullptr);
        wal_mgr_->PutEntries(wal_entries);
    }
}

void TxnManager::AddDeltaEntry(UniquePtr<CatalogDeltaEntry> delta_entry) {
    // Check if the is_running_ is true
    if (is_running_.load() == false) {
        String error_message = "TxnManager is not running, cannot add delta entry";
        UnrecoverableError(error_message);
    }
    bg_task_processor_->Submit(MakeShared<AddDeltaEntryTask>(std::move(delta_entry)));
}

void TxnManager::Start() {
    is_running_.store(true, std::memory_order::relaxed);
    LOG_INFO("TxnManager is started.");
}

void TxnManager::Stop() {
    bool expected = true;
    bool changed = is_running_.compare_exchange_strong(expected, false);
    if (!changed) {
        LOG_INFO("TxnManager::Stop already stopped");
        return;
    }

    LOG_INFO("Txn manager is stopping...");
    std::unique_lock<std::mutex> w_locker(locker_);
    auto it = txn_map_.begin();
    while (it != txn_map_.end()) {
        // remove and notify the wal manager condition variable
        Txn *txn_ptr = it->second.get();
        if (txn_ptr != nullptr) {
            txn_ptr->CancelCommitBottom();
        }
        ++it;
    }
    txn_map_.clear();
    LOG_INFO("TxnManager is stopped");
}

bool TxnManager::Stopped() { return !is_running_.load(); }

TxnTimeStamp TxnManager::CommitTxn(Txn *txn) {
    TxnTimeStamp commit_ts = txn->Commit();
    this->FinishTxn(txn);
    return commit_ts;
}

void TxnManager::RollBackTxn(Txn *txn) {
    txn->Rollback();
    this->FinishTxn(txn);
}

SizeT TxnManager::ActiveTxnCount() {
    std::unique_lock w_lock(locker_);
    return txn_map_.size();
}

Vector<TxnInfo> TxnManager::GetTxnInfoArray() const {
    Vector<TxnInfo> res;

    std::unique_lock w_lock(locker_);
    res.reserve(txn_map_.size());

    for (const auto &txn_pair : txn_map_) {
        TxnInfo txn_info;
        txn_info.txn_id_ = txn_pair.first;
        txn_info.txn_text_ = txn_pair.second->GetTxnText();
        res.emplace_back(txn_info);
    }
    return res;
}

UniquePtr<TxnInfo> TxnManager::GetTxnInfoByID(TransactionID txn_id) const {
    std::unique_lock w_lock(locker_);
    auto iter = txn_map_.find(txn_id);
    if (iter == txn_map_.end()) {
        return nullptr;
    }
    return MakeUnique<TxnInfo>(iter->first, iter->second->GetTxnText());
}

TxnTimeStamp TxnManager::CurrentTS() const { return start_ts_; }

TxnTimeStamp TxnManager::GetNewTimeStamp() { return start_ts_++; }

TxnTimeStamp TxnManager::GetCleanupScanTS() {
    std::lock_guard guard(locker_);
    TxnTimeStamp first_uncommitted_begin_ts = start_ts_;
    if (!begin_txn_by_ts_.empty()) {
        first_uncommitted_begin_ts = begin_txn_by_ts_.begin()->first;
    }
    TxnTimeStamp checkpointed_ts = wal_mgr_->GetCheckpointedTS();
    TxnTimeStamp res = std::min(first_uncommitted_begin_ts, checkpointed_ts);
    LOG_INFO(fmt::format("Cleanup scan ts: {}, checkpoint ts: {}", res, checkpointed_ts));
    for (auto *txn : finished_txns_) {
        res = std::min(res, txn->BeginTS());
    }
    return res;
}

// A Txn can be deleted when there is no uncommitted txn whose begin is less than the commit ts of the txn
// So maintain the least uncommitted begin ts
void TxnManager::FinishTxn(Txn *txn) {
    auto txn_state = txn->GetTxnState();
    auto txn_type = txn->GetTxnType();

    TxnTimeStamp finished_ts = ++start_ts_;

    std::lock_guard guard(locker_);
    if (txn_state == TxnState::kRollbacked) {
        // Rollback by TXN self
        begin_txn_by_ts_.erase(txn->BeginTS());
        txn_map_.erase(txn->TxnID());

        // If rollback happened before commit phase, finishing_txns won't involve the txn.
        // If rollback happened at the commit phase which only caused by txn conflict, the finishing_txns will involve the txn.
        finishing_txns_.erase(txn);

    } else {
        // Committing status and Write TXN
        switch(txn_type) {
            case TxnType::kRead: {
                // Read only TXN
                begin_txn_by_ts_.erase(txn->BeginTS());
                txn_map_.erase(txn->TxnID());
                return;
            }
            case TxnType::kInvalid: {
                String error_message = "Txn type is invalid";
                UnrecoverableError(error_message);
            }
            case TxnType::kWrite: {
                break;
            }
        }

        if (txn_state == TxnState::kCommitting) {
            txn->SetTxnCommitted(finished_ts);
            finished_txns_.emplace_back(txn);
        } else {
            String error_message = fmt::format("Invalid transaction status: {}", ToString(txn_state));
            UnrecoverableError(error_message);
        }

        SizeT remove_n = finishing_txns_.erase(txn);
        if (remove_n == 0) {
            UnrecoverableError("Txn not found in finishing_txns_");
        }
    }

    TxnTimeStamp oldest_uncommitted_begin_ts = finished_ts;
    while (!begin_txn_by_ts_.empty()) {
        auto first_txn_ts = begin_txn_by_ts_.begin()->first;
        auto first_txn_id = begin_txn_by_ts_.begin()->second;
        auto txn_iter = txn_map_.find(first_txn_id);
        if(txn_iter == txn_map_.end()) {
            UnrecoverableError(fmt::format("Txn: {} not found in txn_map", first_txn_id));
        }
        Txn* first_txn_ptr = txn_iter->second.get();
        auto status = first_txn_ptr->GetTxnState();

        if(status == TxnState::kCommitted) {
            begin_txn_by_ts_.erase(first_txn_ts);
            continue;
        }

        if(status == TxnState::kRollbacked) {
            UnrecoverableError(fmt::format("Rollback Txn: {}, should already be removed before.", first_txn_id));
        }

        if(status == TxnState::kInvalid) {
            UnrecoverableError(fmt::format("Txn: {} has invalid txn state.", first_txn_id));
        }

        oldest_uncommitted_begin_ts = first_txn_ptr->BeginTS();
        break;
    }

    while (!finished_txns_.empty()) {
        auto *finished_txn = finished_txns_.front();
        auto finished_txn_id = finished_txn->TxnID();
        if(finished_txn->CommittedTS() <= oldest_uncommitted_begin_ts) {
            // The finished txn is committed before the oldest commit or rollback TXN ts
            finished_txns_.pop_front();

            // LOG_INFO(fmt::format("Txn: {} is erased", finished_txn_id));
            SizeT remove_n = txn_map_.erase(finished_txn_id);
            if (remove_n == 0) {
                String error_message = fmt::format("Txn: {} not found in txn map", finished_txn_id);
                UnrecoverableError(error_message);
            }
        } else {
            LOG_TRACE(fmt::format("{} finish TS: {}, later than {}", finished_txn_id, finished_txn->CommittedTS(), oldest_uncommitted_begin_ts));
            break;
        }
    }
}

bool TxnManager::InCheckpointProcess(TxnTimeStamp commit_ts) {
    std::lock_guard guard(locker_);
    if (commit_ts > ckp_begin_ts_) {
        LOG_TRACE(fmt::format("Full checkpoint begin in {}, cur txn commit_ts: {}, swap to new wal file", ckp_begin_ts_, commit_ts));
        ckp_begin_ts_ = UNCOMMIT_TS;
        return true;
    }
    return false;
}

} // namespace infinity
