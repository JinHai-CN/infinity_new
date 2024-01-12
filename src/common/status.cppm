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

import stl;

export module status;

namespace infinity {

export enum class ErrorCode : long {

    kOk = 0, // success
    kError,
    kNotFound,
    kNotImplemented,
    kReachQueryMemoryLimit,
    kNotSupportedCast,
    kAlreadyExist,
    kNetworkError,
    kServiceUnavailable,
    kWWConflict,
    kDuplicate,
    kUndefined,
    kInvalidIdentifier,

    // 1. config error,

    // 2. Auth error
    kWrongPasswd = 2001,
    kInsufficientPrivilege = 2002,

    // 3. syntax error or access rule violation
    kInvalidUsername = 3001,
    kInvalidPasswd = 3002,
    kInvalidDbName = 3003,
    kInvalidTableName = 3004,
    kInvalidColumnName = 3005,
    kInvalidIndexName = 3006,
    kInvalidColumnDefinition = 3007,
    kInvalidTableDefinition = 3008,
    kInvalidIndexDefinition = 3009,
    kInvalidDataTypeMismatch = 3010,
    kNameTooLong = 3011,
    kReservedName = 3012,
    kSyntaxError = 3013,
    kInvalidParameterValue = 3014,
    kDuplicateUser = 3015,
    kDuplicateDatabaseName = 3016,
    kDuplicateTableName = 3017,
    kDuplicateIndexName = 3018,
    kDuplicateIndexOnColumn = 3019,
    kNoSuchUser = 3020,
    kDBNotExist = 3021,
    kTableNotExist = 3022,
    kIndexNotExist = 3023,
    kColumnNotExist = 3024,
    kAggNotAllowInWhereClause = 3025,
    kColumnInSelectNotInGroupBy = 3026,
    kNoSuchSystemVar = 3027,
    kInvalidSystemVarValue = 3028,
    kSystemVarReadOnly = 3029,

    // 4. Txn fail
    kTxnRollback = 4001,
    kTxnConflict = 4002,

    // 5. Insufficient resource or exceed limits
    kDiskFull = 5001,
    kOutOfMemory = 5002,
    kTooManyConnections = 5003,
    kConfigurationLimitExceed = 5004,
    kQueryIsTooComplex = 5005,

    // 6. Query intervention
    kQueryCancelled = 6001,
    kQueryNotSupported = 6002,

    // 7. System error
    kIOError = 7001,
    kDuplicatedFile = 7002,
    kConfigFileError = 7003,
    kLockFileExists = 7004,
    kCatalogCorrupted = 7005,
    kDataCorrupted = 7006,
    kIndexCorrupted = 7007,
    kFileNotFound = 7008,
    kDirNotFound = 7009,
};

export class Status {
public:
    // 0. Success
    static Status OK() { return {}; }

    // 2. Auth error
    static Status WrongPasswd(const String &user_name);
    static Status InsufficientPrivilege(const String& user_name, const String &detailed_error);

    // 3. Syntax error or access rule violation
    static Status InvalidUserName(const String &user_name);
    static Status InvalidPasswd();
    static Status InvalidDBName(const String &db_name);
    static Status InvalidTableName(const String &table_name);
    static Status InvalidColumnName(const String &column_name);
    static Status InvalidIndexName(const String &index_name);
    static Status DataTypeMismatch(const String &type1, const String &type2);
    static Status NameTooLong(const String &name, const String &object_type);
    static Status ReservedName(const String &name);
    static Status SyntaxError(const String &query_text);
    static Status InvalidParameterValue(const String &parameter_name, const String &parameter_value, const String &recommend_value);
    static Status DuplicateUserName(const String &user_name);
    static Status DuplicateDatabase(const String &db_name);
    static Status DuplicateTable(const String &table_name);
    static Status DuplicateIndex(const String &index_name);
    static Status DuplicateIndexOnColumn(const String &table_name, const String &column_name);
    static Status NoUser(const String &user_name);
    static Status DBNotExist(const String &db_name);
    static Status TableNotExist(const String &table_name);
    static Status IndexNotExist(const String &index_name);
    static Status ColumnNotExist(const String &column_name);
    static Status AggNotAllowInWhere(const String &func_name);
    static Status ColumnInSelectNotInGroupBy(const String &column_name);
    static Status NoSysVar(const String &variable_name);
    static Status SetInvalidVarValue(const String &variable_name, const String &valid_value_range);
    static Status ReadOnlySysVar(const String &sys_var);

    // 4. TXN fail
    static Status TxnRollback(u64 txn_id);
    static Status TxnConflict(u64 txn_id, const String &conflict_reason);

    // 5. Insufficient resource or exceed limits
    static Status DiskFull(const String &detailed_info);
    static Status OutOfMemory(const String &detailed_info);
    static Status TooManyConnections(const String &detailed_info);
    static Status ConfigurationLimitExceed(const String &config_name, const String &config_value, const String &valid_value_range);
    static Status QueryTooBig(const String &query_text, u64 ast_node);

    // 6. Operation intervention
    static Status QueryCancelled(const String &query_text);
    static Status QueryNotSupported(const String &query_text, const String& detailed_reason);

    // 7. System error
    static Status IOError(const String &detailed_info);
    static Status DuplicatedFile(const String &filename);
    static Status ConfigFileError(const String &path, const String &detailed_info);
    static Status LockFileExists(const String &path);
    static Status CatalogCorrupted(const String &path);
    static Status DataCorrupted(const String &path);
    static Status IndexCorrupted(const String &path);
    static Status FileNotFound(const String &path);
    static Status DirNotFound(const String &path);

public:
    Status() = default;

    inline explicit Status(ErrorCode code) : code_(code) {}

    inline Status(ErrorCode code, UniquePtr<String> message) : code_(code), msg_(std::move(message)) {}

    Status(ErrorCode code, const char *msg);

    Status(Status &s);

    Status(Status &&s) noexcept;

    Status &operator=(Status &s) noexcept;

    Status &operator=(Status &&s) noexcept;

    const Status &operator=(const Status &s) noexcept = delete;

    const Status &operator=(const Status &&s) noexcept = delete;

    [[nodiscard]] bool ok() const { return code_ == ErrorCode::kOk && msg_.get() == nullptr; }

    [[nodiscard]] ErrorCode code() const { return code_; }

    void Init(ErrorCode code, const char *msg);

    [[nodiscard]] const char *message() const {
        if (msg_.get() != nullptr) {
            return msg_->c_str();
        } else {
            return nullptr;
        }
    }

    void MoveStatus(Status &s);
    void MoveStatus(Status &&s);

    ErrorCode code_{ErrorCode::kOk};
    UniquePtr<String> msg_{};
};

} // namespace infinity
