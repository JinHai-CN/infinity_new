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

#include <iostream>

module infinity;

import stl;
import logger;
import config;
import resource_manager;
import task_scheduler;
import storage;
import local_file_system;
import third_party;
import query_options;
import query_result;
import database;
import infinity_context;
import session;
import session_manager;
import query_context;
import parser;
import status;

namespace infinity {

std::shared_ptr<Infinity> Infinity::RemoteConnect() {
    std::shared_ptr<Infinity> infinity_ptr = std::make_shared<Infinity>();
    SessionManager* session_mgr = InfinityContext::instance().session_manager();
    infinity_ptr->session_ = session_mgr->CreateRemoteSession();
    return infinity_ptr;
}

void Infinity::RemoteDisconnect() {
    SessionManager* session_mgr = InfinityContext::instance().session_manager();
    session_mgr->RemoveSessionByID(session_->session_id());
    session_.reset();
}

u64 Infinity::GetSessionId() {
    return session_->session_id();
}

void Infinity::LocalInit(const std::string &path) {
    LocalFileSystem fs;
    if (!fs.Exists(path)) {
        std::cerr << path << " doesn't exist." << std::endl;
        return;
    }

    std::shared_ptr<std::string> config_path = std::make_shared<std::string>(path + "/infinity_conf.toml");

    InfinityContext::instance().Init(config_path);
}

void Infinity::LocalUnInit() {
    InfinityContext::instance().UnInit();
}

std::shared_ptr<Infinity> Infinity::LocalConnect() {
    std::shared_ptr<Infinity> infinity_ptr = std::make_shared<Infinity>();

    SessionManager* session_mgr = InfinityContext::instance().session_manager();
    infinity_ptr->session_ = session_mgr->CreateLocalSession();
    return infinity_ptr;
}

void Infinity::LocalDisconnect() {
//    Printf("To disconnect the database.\n");
}

QueryResult Infinity::CreateDatabase(const std::string &db_name, const CreateDatabaseOptions &create_db_options) {
    QueryResult query_result;
    if(db_name.empty()) {
        query_result.result_table_ = nullptr;
        std::unique_ptr<std::string> err_msg = std::make_unique<std::string>("Empty database name is given.");
        LOG_ERROR(*err_msg);
        query_result.status_ = Status(ErrorCode::kError, Move(err_msg));
        return query_result;
    }

    std::unique_ptr<QueryContext> query_context_ptr = std::make_unique<QueryContext>(session_.get());
    query_context_ptr->Init(InfinityContext::instance().config(),
                            InfinityContext::instance().task_scheduler(),
                            InfinityContext::instance().storage(),
                            InfinityContext::instance().resource_manager(),
                            InfinityContext::instance().session_manager());
    std::unique_ptr<CreateStatement> create_statement = std::make_unique<CreateStatement>();
    std::shared_ptr<CreateSchemaInfo> create_schema_info = std::make_shared<CreateSchemaInfo>();
    create_schema_info->schema_name_ = db_name;
    create_statement->create_info_ = create_schema_info;

    create_statement->create_info_->conflict_type_ = create_db_options.conflict_type_;
    query_result = query_context_ptr->QueryStatement(create_statement.get());
    return query_result;
}

QueryResult Infinity::DropDatabase(const std::string &db_name, const DropDatabaseOptions &) {
    std::unique_ptr<QueryContext> query_context_ptr = std::make_unique<QueryContext>(session_.get());
    query_context_ptr->Init(InfinityContext::instance().config(),
                            InfinityContext::instance().task_scheduler(),
                            InfinityContext::instance().storage(),
                            InfinityContext::instance().resource_manager(),
                            InfinityContext::instance().session_manager());
    std::unique_ptr<DropStatement> drop_statement = std::make_unique<DropStatement>();
    std::shared_ptr<DropSchemaInfo> drop_schema_info = std::make_shared<DropSchemaInfo>();
    drop_schema_info->schema_name_ = db_name;
    drop_statement->drop_info_ = drop_schema_info;
    QueryResult result = query_context_ptr->QueryStatement(drop_statement.get());
    return result;
}

QueryResult Infinity::ListDatabases() {
    std::unique_ptr<QueryContext> query_context_ptr = std::make_unique<QueryContext>(session_.get());
    query_context_ptr->Init(InfinityContext::instance().config(),
                            InfinityContext::instance().task_scheduler(),
                            InfinityContext::instance().storage(),
                            InfinityContext::instance().resource_manager(),
                            InfinityContext::instance().session_manager());
    std::unique_ptr<ShowStatement> show_statement = std::make_unique<ShowStatement>();
    show_statement->show_type_ = ShowStmtType::kDatabases;
    QueryResult result = query_context_ptr->QueryStatement(show_statement.get());
    return result;
}

std::shared_ptr<Database> Infinity::GetDatabase(const std::string &db_name) {
    std::unique_ptr<QueryContext> query_context_ptr = std::make_unique<QueryContext>(session_.get());
    query_context_ptr->Init(InfinityContext::instance().config(),
                            InfinityContext::instance().task_scheduler(),
                            InfinityContext::instance().storage(),
                            InfinityContext::instance().resource_manager(),
                            InfinityContext::instance().session_manager());
    std::unique_ptr<CommandStatement> command_statement = std::make_unique<CommandStatement>();
    command_statement->command_info_ = std::make_shared<UseCmd>(db_name.c_str());
    QueryResult result = query_context_ptr->QueryStatement(command_statement.get());
    if (result.status_.ok()) {
        return std::make_shared<Database>(db_name, session_);
    } else {
        return nullptr;
    }
}

QueryResult Infinity::Query(const std::string &query_text) {
    std::unique_ptr<QueryContext> query_context_ptr = std::make_unique<QueryContext>(session_.get());
    query_context_ptr->Init(InfinityContext::instance().config(),
                            InfinityContext::instance().task_scheduler(),
                            InfinityContext::instance().storage(),
                            InfinityContext::instance().resource_manager(),
                            InfinityContext::instance().session_manager());
    QueryResult result = query_context_ptr->Query(query_text);
    return result;
}

QueryResult Infinity::Flush() {
    std::unique_ptr<QueryContext> query_context_ptr = std::make_unique<QueryContext>(session_.get());
    query_context_ptr->Init(InfinityContext::instance().config(),
                            InfinityContext::instance().task_scheduler(),
                            InfinityContext::instance().storage(),
                            InfinityContext::instance().resource_manager(),
                            InfinityContext::instance().session_manager());
    std::unique_ptr<FlushStatement> flush_statement = std::make_unique<FlushStatement>();
    flush_statement->type_ = FlushType::kData;

    QueryResult result = query_context_ptr->QueryStatement(flush_statement.get());
    return result;
}

} // namespace infinity
