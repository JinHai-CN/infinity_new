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

export module table;

import std;
import query_options;
import value;
import parser;
import session;
import query_result;

namespace infinity {

export class Table {
public:
    Table(std::string table_name, std::shared_ptr<BaseSession> session) : table_name_(std::move(table_name)), session_(std::move(session)) {}

    QueryResult CreateIndex(const std::string &index_name, std::vector<IndexInfo *> *index_info_list, CreateIndexOptions create_index_options);

    QueryResult DropIndex(const std::string &index_name);

    QueryResult Insert(std::vector<std::string> *columns, std::vector<std::vector<ParsedExpr *> *> *values);

    QueryResult Import(const std::string &path, ImportOptions import_options);

    QueryResult Delete(ParsedExpr *filter);

    QueryResult Update(ParsedExpr *filter, std::vector<UpdateExpr *> *update_list);

    QueryResult Search(SearchExpr *search_expr, ParsedExpr *filter, std::vector<ParsedExpr *> *output_columns);

private:
    std::string table_name_{};
    std::shared_ptr<BaseSession> session_{};
};

} // namespace infinity
