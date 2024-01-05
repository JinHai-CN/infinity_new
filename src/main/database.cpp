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

module database;

import stl;
import query_options;
import query_result;
import table;
import infinity_context;
import query_context;
import parser;

namespace infinity {

QueryResult Database::CreateTable(const std::string &table_name,
                                  Vector<ColumnDef *> column_defs,
                                  Vector<TableConstraint *> constraints,
                                  const CreateTableOptions &) {
    std::unique_ptr<QueryContext> query_context_ptr = std::make_unique<QueryContext>(session_.get());
    query_context_ptr->Init(InfinityContext::instance().config(),
                            InfinityContext::instance().task_scheduler(),
                            InfinityContext::instance().storage(),
                            InfinityContext::instance().resource_manager(),
                            InfinityContext::instance().session_manager());
    std::unique_ptr<CreateStatement> create_statement = std::make_unique<CreateStatement>();
    std::shared_ptr<CreateTableInfo> create_table_info = std::make_shared<CreateTableInfo>();
    create_table_info->table_name_ = table_name;
    create_table_info->column_defs_ = Move(column_defs);
    create_table_info->constraints_ = Move(constraints);
    create_statement->create_info_ = Move(create_table_info);
    QueryResult result = query_context_ptr->QueryStatement(create_statement.get());
    return result;
}

QueryResult Database::DropTable(const std::string &table_name, const DropTableOptions &options) {
    std::unique_ptr<QueryContext> query_context_ptr = std::make_unique<QueryContext>(session_.get());
    query_context_ptr->Init(InfinityContext::instance().config(),
                            InfinityContext::instance().task_scheduler(),
                            InfinityContext::instance().storage(),
                            InfinityContext::instance().resource_manager(),
                            InfinityContext::instance().session_manager());
    std::unique_ptr<DropStatement> drop_statement = std::make_unique<DropStatement>();
    std::shared_ptr<DropTableInfo> drop_table_info = std::make_shared<DropTableInfo>();
    drop_table_info->schema_name_ = db_name_;
    drop_table_info->table_name_ = table_name;
    drop_table_info->conflict_type_ = options.conflict_type_;
    drop_statement->drop_info_ = drop_table_info;
    QueryResult result = query_context_ptr->QueryStatement(drop_statement.get());
    return result;
}

QueryResult Database::ListTables() {
    std::unique_ptr<QueryContext> query_context_ptr = std::make_unique<QueryContext>(session_.get());
    query_context_ptr->Init(InfinityContext::instance().config(),
                            InfinityContext::instance().task_scheduler(),
                            InfinityContext::instance().storage(),
                            InfinityContext::instance().resource_manager(),
                            InfinityContext::instance().session_manager());
    std::unique_ptr<ShowStatement> show_statement = std::make_unique<ShowStatement>();
    show_statement->show_type_ = ShowStmtType::kTables;
    QueryResult result = query_context_ptr->QueryStatement(show_statement.get());
    return result;
}

QueryResult Database::DescribeTable(const std::string &) {
    std::unique_ptr<QueryContext> query_context_ptr = std::make_unique<QueryContext>(session_.get());
    query_context_ptr->Init(InfinityContext::instance().config(),
                            InfinityContext::instance().task_scheduler(),
                            InfinityContext::instance().storage(),
                            InfinityContext::instance().resource_manager(),
                            InfinityContext::instance().session_manager());
    std::unique_ptr<ShowStatement> show_statement = std::make_unique<ShowStatement>();
    show_statement->show_type_ = ShowStmtType::kColumns;
    QueryResult result = query_context_ptr->QueryStatement(show_statement.get());
    return result;
}

std::shared_ptr<Table> Database::GetTable(const std::string &table_name) {
    std::unique_ptr<QueryContext> query_context_ptr = std::make_unique<QueryContext>(session_.get());
    query_context_ptr->Init(InfinityContext::instance().config(),
                            InfinityContext::instance().task_scheduler(),
                            InfinityContext::instance().storage(),
                            InfinityContext::instance().resource_manager(),
                            InfinityContext::instance().session_manager());
    std::unique_ptr<CommandStatement> command_statement = std::make_unique<CommandStatement>();
    command_statement->command_info_ = std::make_shared<CheckTable>(table_name.c_str());
    QueryResult result= query_context_ptr->QueryStatement(command_statement.get());
    if (result.status_.ok()) {
        return std::make_shared<Table>(table_name, session_);
    } else {
        return nullptr;
    }
}

} // namespace infinity
