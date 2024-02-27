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

export module periodic_trigger;

import stl;
import background_process;
import catalog;
import txn_manager;

namespace infinity {

export class PeriodicTrigger {
public:
    explicit PeriodicTrigger(std::chrono::milliseconds interval) : interval_(interval), last_check_(std::chrono::system_clock::now()) {}

    virtual ~PeriodicTrigger() = default;

    bool Check() {
        const auto now = std::chrono::system_clock::now();
        if (now - last_check_ < interval_) {
            return false;
        }
        last_check_ = now;
        return true;
    }

    virtual void Trigger() = 0;

    void Reset() { last_check_ = std::chrono::system_clock::now(); }

private:
    const std::chrono::milliseconds interval_;
    std::chrono::system_clock::time_point last_check_;
};

export class CleanupPeriodicTrigger final : public PeriodicTrigger {
public:
    CleanupPeriodicTrigger(std::chrono::milliseconds interval, BGTaskProcessor *bg_processor, Catalog *catalog, TxnManager *txn_mgr)
        : PeriodicTrigger(interval), bg_processor_(bg_processor), catalog_(catalog), txn_mgr_(txn_mgr) {}

    virtual void Trigger() override;

private:
    BGTaskProcessor *const bg_processor_{};
    Catalog *const catalog_{};
    TxnManager *const txn_mgr_{};

    TxnTimeStamp last_visible_ts_{0};
};

} // namespace infinity
