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

#include <cstring>

module status;

import stl;
import third_party;

namespace infinity {

Status::Status(ErrorCode code, const char *msg) : code_(code) { msg_ = MakeUnique<String>(msg); }

Status::Status(Status &s) { MoveStatus(s); }

Status::Status(Status &&s) noexcept { MoveStatus(s); }

Status &Status::operator=(Status &s) noexcept {
    MoveStatus(s);
    return *this;
}

Status &Status::operator=(Status &&s) noexcept {
    MoveStatus(s);
    return *this;
}

void Status::MoveStatus(Status &s) {
    code_ = s.code_;
    msg_ = std::move(s.msg_);
}

void Status::MoveStatus(Status &&s) {
    code_ = s.code_;
    msg_ = std::move(s.msg_);
    s.msg_ = nullptr;
}

void Status::Init(ErrorCode code, const char *msg) {
    if (msg_.get() != nullptr) {
        msg_.reset();
    }
    msg_ = MakeUnique<String>(msg);
    code_ = code;
}

void Status::AppendMessage(const String& msg) {
    if (msg_.get() != nullptr) {
        msg_->append(msg);
    } else {
        msg_ = MakeUnique<String>(msg);
    }
}

// Error functions
// 2. Auth error
Status Status::WrongPasswd(const String &user_name) {
    return Status(ErrorCode::kWrongPasswd, MakeUnique<String>(fmt::format("Invalid password to login user: {}", user_name)));
}
Status Status::InsufficientPrivilege(const String &user_name, const String &detailed_error) {
    return Status(ErrorCode::kInsufficientPrivilege, MakeUnique<String>(fmt::format("{} do not have permission to {}", user_name, detailed_error)));
}

// 3. Syntax error or access rule violation
Status Status::InvalidUserName(const String &user_name) {
    return Status(ErrorCode::kInvalidUsername, MakeUnique<String>(fmt::format("{} is a invalid user name", user_name)));
}

Status Status::InvalidPasswd() { return Status(ErrorCode::kInvalidPasswd, MakeUnique<String>(fmt::format("Invalid password"))); }

Status Status::InvalidDBName(const String &db_name) {
    return Status(ErrorCode::kInvalidDbName, MakeUnique<String>(fmt::format("{} is a invalid database name", db_name)));
}

Status Status::InvalidTableName(const String &table_name) {
    return Status(ErrorCode::kInvalidTableName, MakeUnique<String>(fmt::format("{} is a invalid table name", table_name)));
}

Status Status::InvalidColumnName(const String &column_name) {
    return Status(ErrorCode::kInvalidColumnName, MakeUnique<String>(fmt::format("{} is a invalid column name", column_name)));
}

Status Status::InvalidIndexName(const String &index_name) {
    return Status(ErrorCode::kInvalidIndexName, MakeUnique<String>(fmt::format("{} is a invalid index name", index_name)));
}

Status Status::DataTypeMismatch(const String &type1, const String &type2) {
    return Status(ErrorCode::kInvalidDataTypeMismatch, MakeUnique<String>(fmt::format("Expected: {}, but {} is given.", type1, type2)));
}
Status Status::NameTooLong(const String &name, const String &object_type) {
    return Status(ErrorCode::kNameTooLong, MakeUnique<String>(fmt::format("{} is too long for a {} name", name, object_type)));
}

Status Status::ReservedName(const String &name) {
    return Status(ErrorCode::kReservedName, MakeUnique<String>(fmt::format("{} is a reserved name", name)));
}

Status Status::SyntaxError(const String &query_text) {
    return Status(ErrorCode::kSyntaxError, MakeUnique<String>(fmt::format("Invalid syntax: {}", query_text)));
}

Status Status::InvalidParameterValue(const String &parameter_name, const String &parameter_value, const String &recommend_value) {
    return Status(ErrorCode::kInvalidParameterValue,
                  MakeUnique<String>(fmt::format("{}: {} is invalid, Recommended value: {}", parameter_name, parameter_value, recommend_value)));
}

Status Status::DuplicateUserName(const String &user_name) {
    return Status(ErrorCode::kDuplicateUser, MakeUnique<String>(fmt::format("User: {} is already existed", user_name)));
}

Status Status::DuplicateDatabase(const String &db_name) {
    return Status(ErrorCode::kDuplicateDatabaseName, MakeUnique<String>(fmt::format("Database: {} is already existed", db_name)));
}

Status Status::DuplicateTable(const String &table_name) {
    return Status(ErrorCode::kDuplicateTableName, MakeUnique<String>(fmt::format("Table: {} is already existed", table_name)));
}

Status Status::DuplicateIndex(const String &index_name) {
    return Status(ErrorCode::kDuplicateIndexName, MakeUnique<String>(fmt::format("Index: {} is already existed", index_name)));
}

Status Status::DuplicateIndexOnColumn(const String &table_name, const String &column_name) {
    return Status(ErrorCode::kDuplicateIndexOnColumn,
                  MakeUnique<String>(fmt::format("Index: {} is created on column : {} with same parameters", table_name, column_name)));
}

Status Status::NoUser(const String &user_name) {
    return Status(ErrorCode::kNoSuchUser, MakeUnique<String>(fmt::format("No such user: {}", user_name)));
}

Status Status::DBNotExist(const String &db_name) {
    return Status(ErrorCode::kDBNotExist, MakeUnique<String>(fmt::format("Database: {} doesn't exist.", db_name)));
}

Status Status::TableNotExist(const String &table_name) {
    return Status(ErrorCode::kTableNotExist, MakeUnique<String>(fmt::format("Table: {} doesn't exist.", table_name)));
}

Status Status::IndexNotExist(const String &index_name) {
    return Status(ErrorCode::kIndexNotExist, MakeUnique<String>(fmt::format("Index: {} doesn't exist.", index_name)));
}

Status Status::ColumnNotExist(const String &column_name) {
    return Status(ErrorCode::kColumnNotExist, MakeUnique<String>(fmt::format("Column: {} doesn't exist", column_name)));
}

Status Status::AggNotAllowInWhere(const String &func_name) {
    return Status(ErrorCode::kAggNotAllowInWhereClause,
                  MakeUnique<String>(fmt::format("Aggregate function: {} isn't allowed in where clause", func_name)));
}

Status Status::ColumnInSelectNotInGroupBy(const String &column_name) {
    return Status(ErrorCode::kColumnInSelectNotInGroupBy,
                  MakeUnique<String>(fmt::format("Column: {} must appear in the GROUP BY clause or be used in an aggregated function", column_name)));
}

Status Status::NoSysVar(const String &variable_name) {
    return Status(ErrorCode::kNoSuchSystemVar, MakeUnique<String>(fmt::format("No such system variable: {}", variable_name)));
}

Status Status::SetInvalidVarValue(const String &variable_name, const String &valid_value_range) {
    return Status(ErrorCode::kInvalidSystemVarValue, MakeUnique<String>(fmt::format("{} value range is {}", variable_name, valid_value_range)));
}

Status Status::ReadOnlySysVar(const String &sys_var) {
    return Status(ErrorCode::kSystemVarReadOnly, MakeUnique<String>(fmt::format("{} is read-only", sys_var)));
}

Status Status::FunctionNotFound(const String &function_name) {
    return Status(ErrorCode::kFunctionNotFound, MakeUnique<String>(fmt::format("{} not found", function_name)));
}

Status Status::SpecialFunctionNotFound() {
    return Status(ErrorCode::kSpecialFunctionNotFound);
}

// 4. TXN fail
Status Status::TxnRollback(u64 txn_id) {
    return Status(ErrorCode::kTxnRollback, MakeUnique<String>(fmt::format("Transaction: {} is rollback", txn_id)));
}
Status Status::TxnConflict(u64 txn_id, const String &conflict_reason) {
    return Status(ErrorCode::kTxnConflict,
                  MakeUnique<String>(fmt::format("Transaction: {} is conflicted, detailed info: {}", txn_id, conflict_reason)));
}

// 5. Insufficient resource or exceed limits
Status Status::DiskFull(const String &detailed_info) {
    return Status(ErrorCode::kDiskFull, MakeUnique<String>(fmt::format("Disk full: {}", detailed_info)));
}

Status Status::OutOfMemory(const String &detailed_info) {
    return Status(ErrorCode::kOutOfMemory, MakeUnique<String>(fmt::format("Out of memory: {}", detailed_info)));
}

Status Status::TooManyConnections(const String &detailed_info) {
    return Status(ErrorCode::kTooManyConnections, MakeUnique<String>(fmt::format("Too many connections, {}", detailed_info)));
}

Status Status::ConfigurationLimitExceed(const String &config_name, const String &config_value, const String &valid_value_range) {
    return Status(ErrorCode::kConfigurationLimitExceed,
                  MakeUnique<String>(
                      fmt::format("Set {} value: {} failed, {} value valid range : {}", config_name, config_value, config_name, valid_value_range)));
}

Status Status::QueryTooBig(const String &query_text, u64 ast_node) {
    return Status(ErrorCode::kQueryIsTooComplex, MakeUnique<String>(fmt::format("Query: {} is too complex with {} AST nodes", query_text, ast_node)));
}

// 6. Operation intervention
Status Status::QueryCancelled(const String &query_text) {
    return Status(ErrorCode::kQueryCancelled, MakeUnique<String>(fmt::format("Query: {} is cancelled", query_text)));
}

Status Status::QueryNotSupported(const String &query_text, const String &detailed_reason) {
    return Status(ErrorCode::kQueryNotSupported, MakeUnique<String>(fmt::format("Query: {} isn't supported, {}", query_text, detailed_reason)));
}

Status Status::ClientClose() {
    return Status(ErrorCode::kClientClose);
}

// 7. System error
Status Status::IOError(const String &detailed_info) {
    return Status(ErrorCode::kIOError, MakeUnique<String>(fmt::format("IO error: {}", detailed_info)));
}

Status Status::DuplicatedFile(const String &filename) {
    return Status(ErrorCode::kDuplicatedFile, MakeUnique<String>(fmt::format("Duplicated file: {}", filename)));
}

Status Status::ConfigFileError(const String &path, const String &detailed_info) {
    return Status(ErrorCode::kConfigFileError, MakeUnique<String>(fmt::format("Config file: {}, {}", path, detailed_info)));
}

Status Status::LockFileExists(const String &path) {
    return Status(ErrorCode::kLockFileExists, MakeUnique<String>(fmt::format("Lock file: is existed", path)));
}

Status Status::CatalogCorrupted(const String &path) {
    return Status(ErrorCode::kCatalogCorrupted, MakeUnique<String>(fmt::format("Catalog: {} is corrupted", path)));
}

Status Status::DataCorrupted(const String &path) {
    return Status(ErrorCode::kDataCorrupted, MakeUnique<String>(fmt::format("Data: {} is corrupted", path)));
}

Status Status::IndexCorrupted(const String &path) {
    return Status(ErrorCode::kIndexCorrupted, MakeUnique<String>(fmt::format("Index: {} is corrupted", path)));
}

Status Status::FileNotFound(const String &path) {
    return Status(ErrorCode::kFileNotFound, MakeUnique<String>(fmt::format("File: {} isn't found", path)));
}

Status Status::DirNotFound(const String &path) {
    return Status(ErrorCode::kDirNotFound, MakeUnique<String>(fmt::format("Directory: {} isn't found", path)));
}

} // namespace infinity
