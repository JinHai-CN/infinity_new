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

export module infinity;

import std;
import type_alias;
import config;
import resource_manager;
import task_scheduler;
import storage;
import database;
import query_result;
import query_options;
import infinity_context;
import session;

namespace infinity {

export class Infinity {
public:
    Infinity() = default;

    ~Infinity() = default;

    static std::shared_ptr<Infinity> RemoteConnect();

    u64 GetSessionId();

    void RemoteDisconnect();

    static void LocalInit(const std::string &path);

    static void LocalUnInit();

    static std::shared_ptr<Infinity> LocalConnect();

    void LocalDisconnect();

    QueryResult CreateDatabase(const std::string &db_name, const CreateDatabaseOptions &options);

    QueryResult DropDatabase(const std::string &db_name, const DropDatabaseOptions &options);

    QueryResult ListDatabases();

    std::shared_ptr<Database> GetDatabase(const std::string &db_name);

    QueryResult Flush();

    // For embedded sqllogictest
    QueryResult Query(const std::string& query_text);

private:
    std::shared_ptr<BaseSession> session_{};
};

} // namespace infinity
