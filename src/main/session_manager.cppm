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

import std;
import type_alias;
import session;

export module session_manager;

namespace infinity {

export class SessionManager {

public:
    SessionManager() = default;

    std::shared_ptr<RemoteSession> CreateRemoteSession() {
        u64 session_id = ++ session_id_generator_;
        std::shared_ptr<RemoteSession> remote_session = std::make_shared<RemoteSession>(session_id);
        {
            std::unique_lock<std::shared_mutex> w_locker(rw_locker_);
            sessions_.emplace(session_id, remote_session.get());
        }
        return remote_session;
    }

    std::shared_ptr<LocalSession> CreateLocalSession() {
        u64 session_id = ++ session_id_generator_;
        std::shared_ptr<LocalSession> local_session = std::make_shared<LocalSession>(session_id);
        {
            std::unique_lock<std::shared_mutex> w_locker(rw_locker_);
            sessions_.emplace(session_id, local_session.get());
        }
        return local_session;
    }

    BaseSession* GetSessionByID(u64 session_id) {
        std::shared_lock<std::shared_mutex> r_locker(rw_locker_);
        auto iter = sessions_.find(session_id);
        if(iter == sessions_.end()) {
            return nullptr;
        } else {
            return iter->second;
        }
    }

    void RemoveSessionByID(u64 session_id) {
        std::unique_lock<std::shared_mutex> w_locker(rw_locker_);
        sessions_.erase(session_id);
    }

    SizeT GetSessionCount() {
        std::unique_lock<std::shared_mutex> r_locker(rw_locker_);
        return sessions_.size();
    }

private:
    std::shared_mutex rw_locker_{};
    std::unordered_map<u64, BaseSession*> sessions_;

    // First session is ONE;
    std::atomic<u64> session_id_generator_{};
};

}