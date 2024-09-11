/**
 * Autogenerated by Thrift Compiler (0.20.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef peer_server_TYPES_H
#define peer_server_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/TBase.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <functional>
#include <memory>


namespace infinity_peer_server {

struct NodeType {
  enum type {
    kLeader = 0,
    kFollower = 1,
    kLearner = 2
  };
};

extern const std::map<int, const char*> _NodeType_VALUES_TO_NAMES;

std::ostream& operator<<(std::ostream& out, const NodeType::type& val);

std::string to_string(const NodeType::type& val);

class NodeInfo;

class RegisterRequest;

class RegisterResponse;

class UnregisterRequest;

class UnregisterResponse;

class HeartBeatRequest;

class HeartBeatResponse;

class SyncLogRequest;

class SyncLogResponse;

class ChangeRoleRequest;

class ChangeRoleResponse;

class NewLeaderRequest;

class NewLeaderResponse;

typedef struct _NodeInfo__isset {
  _NodeInfo__isset() : node_name(false), node_type(false), node_ip(false), node_port(false), txn_timestamp(false) {}
  bool node_name :1;
  bool node_type :1;
  bool node_ip :1;
  bool node_port :1;
  bool txn_timestamp :1;
} _NodeInfo__isset;

class NodeInfo : public virtual ::apache::thrift::TBase {
 public:

  NodeInfo(const NodeInfo&);
  NodeInfo& operator=(const NodeInfo&);
  NodeInfo() noexcept
           : node_name(),
             node_type(static_cast<NodeType::type>(0)),
             node_ip(),
             node_port(0),
             txn_timestamp(0) {
  }

  virtual ~NodeInfo() noexcept;
  std::string node_name;
  /**
   * 
   * @see NodeType
   */
  NodeType::type node_type;
  std::string node_ip;
  int64_t node_port;
  int64_t txn_timestamp;

  _NodeInfo__isset __isset;

  void __set_node_name(const std::string& val);

  void __set_node_type(const NodeType::type val);

  void __set_node_ip(const std::string& val);

  void __set_node_port(const int64_t val);

  void __set_txn_timestamp(const int64_t val);

  bool operator == (const NodeInfo & rhs) const
  {
    if (!(node_name == rhs.node_name))
      return false;
    if (!(node_type == rhs.node_type))
      return false;
    if (!(node_ip == rhs.node_ip))
      return false;
    if (!(node_port == rhs.node_port))
      return false;
    if (!(txn_timestamp == rhs.txn_timestamp))
      return false;
    return true;
  }
  bool operator != (const NodeInfo &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NodeInfo & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(NodeInfo &a, NodeInfo &b);

std::ostream& operator<<(std::ostream& out, const NodeInfo& obj);

typedef struct _RegisterRequest__isset {
  _RegisterRequest__isset() : node_name(false), node_type(false), node_ip(false), node_port(false), txn_timestamp(false) {}
  bool node_name :1;
  bool node_type :1;
  bool node_ip :1;
  bool node_port :1;
  bool txn_timestamp :1;
} _RegisterRequest__isset;

class RegisterRequest : public virtual ::apache::thrift::TBase {
 public:

  RegisterRequest(const RegisterRequest&);
  RegisterRequest& operator=(const RegisterRequest&);
  RegisterRequest() noexcept
                  : node_name(),
                    node_type(static_cast<NodeType::type>(0)),
                    node_ip(),
                    node_port(0),
                    txn_timestamp(0) {
  }

  virtual ~RegisterRequest() noexcept;
  std::string node_name;
  /**
   * 
   * @see NodeType
   */
  NodeType::type node_type;
  std::string node_ip;
  int64_t node_port;
  int64_t txn_timestamp;

  _RegisterRequest__isset __isset;

  void __set_node_name(const std::string& val);

  void __set_node_type(const NodeType::type val);

  void __set_node_ip(const std::string& val);

  void __set_node_port(const int64_t val);

  void __set_txn_timestamp(const int64_t val);

  bool operator == (const RegisterRequest & rhs) const
  {
    if (!(node_name == rhs.node_name))
      return false;
    if (!(node_type == rhs.node_type))
      return false;
    if (!(node_ip == rhs.node_ip))
      return false;
    if (!(node_port == rhs.node_port))
      return false;
    if (!(txn_timestamp == rhs.txn_timestamp))
      return false;
    return true;
  }
  bool operator != (const RegisterRequest &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const RegisterRequest & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(RegisterRequest &a, RegisterRequest &b);

std::ostream& operator<<(std::ostream& out, const RegisterRequest& obj);

typedef struct _RegisterResponse__isset {
  _RegisterResponse__isset() : error_code(false), error_message(false), leader_name(false), leader_term(false), heart_beat_interval(false) {}
  bool error_code :1;
  bool error_message :1;
  bool leader_name :1;
  bool leader_term :1;
  bool heart_beat_interval :1;
} _RegisterResponse__isset;

class RegisterResponse : public virtual ::apache::thrift::TBase {
 public:

  RegisterResponse(const RegisterResponse&);
  RegisterResponse& operator=(const RegisterResponse&);
  RegisterResponse() noexcept
                   : error_code(0),
                     error_message(),
                     leader_name(),
                     leader_term(0),
                     heart_beat_interval(0) {
  }

  virtual ~RegisterResponse() noexcept;
  int64_t error_code;
  std::string error_message;
  std::string leader_name;
  int64_t leader_term;
  int64_t heart_beat_interval;

  _RegisterResponse__isset __isset;

  void __set_error_code(const int64_t val);

  void __set_error_message(const std::string& val);

  void __set_leader_name(const std::string& val);

  void __set_leader_term(const int64_t val);

  void __set_heart_beat_interval(const int64_t val);

  bool operator == (const RegisterResponse & rhs) const
  {
    if (!(error_code == rhs.error_code))
      return false;
    if (!(error_message == rhs.error_message))
      return false;
    if (!(leader_name == rhs.leader_name))
      return false;
    if (!(leader_term == rhs.leader_term))
      return false;
    if (!(heart_beat_interval == rhs.heart_beat_interval))
      return false;
    return true;
  }
  bool operator != (const RegisterResponse &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const RegisterResponse & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(RegisterResponse &a, RegisterResponse &b);

std::ostream& operator<<(std::ostream& out, const RegisterResponse& obj);

typedef struct _UnregisterRequest__isset {
  _UnregisterRequest__isset() : node_name(false) {}
  bool node_name :1;
} _UnregisterRequest__isset;

class UnregisterRequest : public virtual ::apache::thrift::TBase {
 public:

  UnregisterRequest(const UnregisterRequest&);
  UnregisterRequest& operator=(const UnregisterRequest&);
  UnregisterRequest() noexcept
                    : node_name() {
  }

  virtual ~UnregisterRequest() noexcept;
  std::string node_name;

  _UnregisterRequest__isset __isset;

  void __set_node_name(const std::string& val);

  bool operator == (const UnregisterRequest & rhs) const
  {
    if (!(node_name == rhs.node_name))
      return false;
    return true;
  }
  bool operator != (const UnregisterRequest &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const UnregisterRequest & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(UnregisterRequest &a, UnregisterRequest &b);

std::ostream& operator<<(std::ostream& out, const UnregisterRequest& obj);

typedef struct _UnregisterResponse__isset {
  _UnregisterResponse__isset() : error_code(false), error_message(false) {}
  bool error_code :1;
  bool error_message :1;
} _UnregisterResponse__isset;

class UnregisterResponse : public virtual ::apache::thrift::TBase {
 public:

  UnregisterResponse(const UnregisterResponse&);
  UnregisterResponse& operator=(const UnregisterResponse&);
  UnregisterResponse() noexcept
                     : error_code(0),
                       error_message() {
  }

  virtual ~UnregisterResponse() noexcept;
  int64_t error_code;
  std::string error_message;

  _UnregisterResponse__isset __isset;

  void __set_error_code(const int64_t val);

  void __set_error_message(const std::string& val);

  bool operator == (const UnregisterResponse & rhs) const
  {
    if (!(error_code == rhs.error_code))
      return false;
    if (!(error_message == rhs.error_message))
      return false;
    return true;
  }
  bool operator != (const UnregisterResponse &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const UnregisterResponse & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(UnregisterResponse &a, UnregisterResponse &b);

std::ostream& operator<<(std::ostream& out, const UnregisterResponse& obj);

typedef struct _HeartBeatRequest__isset {
  _HeartBeatRequest__isset() : node_name(false), txn_timestamp(false) {}
  bool node_name :1;
  bool txn_timestamp :1;
} _HeartBeatRequest__isset;

class HeartBeatRequest : public virtual ::apache::thrift::TBase {
 public:

  HeartBeatRequest(const HeartBeatRequest&);
  HeartBeatRequest& operator=(const HeartBeatRequest&);
  HeartBeatRequest() noexcept
                   : node_name(),
                     txn_timestamp(0) {
  }

  virtual ~HeartBeatRequest() noexcept;
  std::string node_name;
  int64_t txn_timestamp;

  _HeartBeatRequest__isset __isset;

  void __set_node_name(const std::string& val);

  void __set_txn_timestamp(const int64_t val);

  bool operator == (const HeartBeatRequest & rhs) const
  {
    if (!(node_name == rhs.node_name))
      return false;
    if (!(txn_timestamp == rhs.txn_timestamp))
      return false;
    return true;
  }
  bool operator != (const HeartBeatRequest &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const HeartBeatRequest & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(HeartBeatRequest &a, HeartBeatRequest &b);

std::ostream& operator<<(std::ostream& out, const HeartBeatRequest& obj);

typedef struct _HeartBeatResponse__isset {
  _HeartBeatResponse__isset() : error_code(false), error_message(false), leader_term(false), other_nodes(false) {}
  bool error_code :1;
  bool error_message :1;
  bool leader_term :1;
  bool other_nodes :1;
} _HeartBeatResponse__isset;

class HeartBeatResponse : public virtual ::apache::thrift::TBase {
 public:

  HeartBeatResponse(const HeartBeatResponse&);
  HeartBeatResponse& operator=(const HeartBeatResponse&);
  HeartBeatResponse() noexcept
                    : error_code(0),
                      error_message(),
                      leader_term(0) {
  }

  virtual ~HeartBeatResponse() noexcept;
  int64_t error_code;
  std::string error_message;
  int64_t leader_term;
  std::vector<NodeInfo>  other_nodes;

  _HeartBeatResponse__isset __isset;

  void __set_error_code(const int64_t val);

  void __set_error_message(const std::string& val);

  void __set_leader_term(const int64_t val);

  void __set_other_nodes(const std::vector<NodeInfo> & val);

  bool operator == (const HeartBeatResponse & rhs) const
  {
    if (!(error_code == rhs.error_code))
      return false;
    if (!(error_message == rhs.error_message))
      return false;
    if (!(leader_term == rhs.leader_term))
      return false;
    if (!(other_nodes == rhs.other_nodes))
      return false;
    return true;
  }
  bool operator != (const HeartBeatResponse &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const HeartBeatResponse & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(HeartBeatResponse &a, HeartBeatResponse &b);

std::ostream& operator<<(std::ostream& out, const HeartBeatResponse& obj);

typedef struct _SyncLogRequest__isset {
  _SyncLogRequest__isset() : node_name(false), log_entries(false) {}
  bool node_name :1;
  bool log_entries :1;
} _SyncLogRequest__isset;

class SyncLogRequest : public virtual ::apache::thrift::TBase {
 public:

  SyncLogRequest(const SyncLogRequest&);
  SyncLogRequest& operator=(const SyncLogRequest&);
  SyncLogRequest() noexcept
                 : node_name() {
  }

  virtual ~SyncLogRequest() noexcept;
  std::string node_name;
  std::vector<std::string>  log_entries;

  _SyncLogRequest__isset __isset;

  void __set_node_name(const std::string& val);

  void __set_log_entries(const std::vector<std::string> & val);

  bool operator == (const SyncLogRequest & rhs) const
  {
    if (!(node_name == rhs.node_name))
      return false;
    if (!(log_entries == rhs.log_entries))
      return false;
    return true;
  }
  bool operator != (const SyncLogRequest &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SyncLogRequest & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(SyncLogRequest &a, SyncLogRequest &b);

std::ostream& operator<<(std::ostream& out, const SyncLogRequest& obj);

typedef struct _SyncLogResponse__isset {
  _SyncLogResponse__isset() : error_code(false), error_message(false), txn_timestamp(false) {}
  bool error_code :1;
  bool error_message :1;
  bool txn_timestamp :1;
} _SyncLogResponse__isset;

class SyncLogResponse : public virtual ::apache::thrift::TBase {
 public:

  SyncLogResponse(const SyncLogResponse&);
  SyncLogResponse& operator=(const SyncLogResponse&);
  SyncLogResponse() noexcept
                  : error_code(0),
                    error_message(),
                    txn_timestamp(0) {
  }

  virtual ~SyncLogResponse() noexcept;
  int64_t error_code;
  std::string error_message;
  int64_t txn_timestamp;

  _SyncLogResponse__isset __isset;

  void __set_error_code(const int64_t val);

  void __set_error_message(const std::string& val);

  void __set_txn_timestamp(const int64_t val);

  bool operator == (const SyncLogResponse & rhs) const
  {
    if (!(error_code == rhs.error_code))
      return false;
    if (!(error_message == rhs.error_message))
      return false;
    if (!(txn_timestamp == rhs.txn_timestamp))
      return false;
    return true;
  }
  bool operator != (const SyncLogResponse &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const SyncLogResponse & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(SyncLogResponse &a, SyncLogResponse &b);

std::ostream& operator<<(std::ostream& out, const SyncLogResponse& obj);

typedef struct _ChangeRoleRequest__isset {
  _ChangeRoleRequest__isset() : node_name(false), node_type(false) {}
  bool node_name :1;
  bool node_type :1;
} _ChangeRoleRequest__isset;

class ChangeRoleRequest : public virtual ::apache::thrift::TBase {
 public:

  ChangeRoleRequest(const ChangeRoleRequest&);
  ChangeRoleRequest& operator=(const ChangeRoleRequest&);
  ChangeRoleRequest() noexcept
                    : node_name(),
                      node_type(static_cast<NodeType::type>(0)) {
  }

  virtual ~ChangeRoleRequest() noexcept;
  std::string node_name;
  /**
   * 
   * @see NodeType
   */
  NodeType::type node_type;

  _ChangeRoleRequest__isset __isset;

  void __set_node_name(const std::string& val);

  void __set_node_type(const NodeType::type val);

  bool operator == (const ChangeRoleRequest & rhs) const
  {
    if (!(node_name == rhs.node_name))
      return false;
    if (!(node_type == rhs.node_type))
      return false;
    return true;
  }
  bool operator != (const ChangeRoleRequest &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ChangeRoleRequest & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(ChangeRoleRequest &a, ChangeRoleRequest &b);

std::ostream& operator<<(std::ostream& out, const ChangeRoleRequest& obj);

typedef struct _ChangeRoleResponse__isset {
  _ChangeRoleResponse__isset() : node_name(false) {}
  bool node_name :1;
} _ChangeRoleResponse__isset;

class ChangeRoleResponse : public virtual ::apache::thrift::TBase {
 public:

  ChangeRoleResponse(const ChangeRoleResponse&);
  ChangeRoleResponse& operator=(const ChangeRoleResponse&);
  ChangeRoleResponse() noexcept
                     : node_name() {
  }

  virtual ~ChangeRoleResponse() noexcept;
  std::string node_name;

  _ChangeRoleResponse__isset __isset;

  void __set_node_name(const std::string& val);

  bool operator == (const ChangeRoleResponse & rhs) const
  {
    if (!(node_name == rhs.node_name))
      return false;
    return true;
  }
  bool operator != (const ChangeRoleResponse &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const ChangeRoleResponse & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(ChangeRoleResponse &a, ChangeRoleResponse &b);

std::ostream& operator<<(std::ostream& out, const ChangeRoleResponse& obj);

typedef struct _NewLeaderRequest__isset {
  _NewLeaderRequest__isset() : node_name(false), node_ip(false), node_port(false), new_node_type(false), new_leader_term(false) {}
  bool node_name :1;
  bool node_ip :1;
  bool node_port :1;
  bool new_node_type :1;
  bool new_leader_term :1;
} _NewLeaderRequest__isset;

class NewLeaderRequest : public virtual ::apache::thrift::TBase {
 public:

  NewLeaderRequest(const NewLeaderRequest&);
  NewLeaderRequest& operator=(const NewLeaderRequest&);
  NewLeaderRequest() noexcept
                   : node_name(),
                     node_ip(),
                     node_port(0),
                     new_node_type(static_cast<NodeType::type>(0)),
                     new_leader_term(0) {
  }

  virtual ~NewLeaderRequest() noexcept;
  std::string node_name;
  std::string node_ip;
  int64_t node_port;
  /**
   * 
   * @see NodeType
   */
  NodeType::type new_node_type;
  int64_t new_leader_term;

  _NewLeaderRequest__isset __isset;

  void __set_node_name(const std::string& val);

  void __set_node_ip(const std::string& val);

  void __set_node_port(const int64_t val);

  void __set_new_node_type(const NodeType::type val);

  void __set_new_leader_term(const int64_t val);

  bool operator == (const NewLeaderRequest & rhs) const
  {
    if (!(node_name == rhs.node_name))
      return false;
    if (!(node_ip == rhs.node_ip))
      return false;
    if (!(node_port == rhs.node_port))
      return false;
    if (!(new_node_type == rhs.new_node_type))
      return false;
    if (!(new_leader_term == rhs.new_leader_term))
      return false;
    return true;
  }
  bool operator != (const NewLeaderRequest &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NewLeaderRequest & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(NewLeaderRequest &a, NewLeaderRequest &b);

std::ostream& operator<<(std::ostream& out, const NewLeaderRequest& obj);

typedef struct _NewLeaderResponse__isset {
  _NewLeaderResponse__isset() : node_name(false) {}
  bool node_name :1;
} _NewLeaderResponse__isset;

class NewLeaderResponse : public virtual ::apache::thrift::TBase {
 public:

  NewLeaderResponse(const NewLeaderResponse&);
  NewLeaderResponse& operator=(const NewLeaderResponse&);
  NewLeaderResponse() noexcept
                    : node_name() {
  }

  virtual ~NewLeaderResponse() noexcept;
  std::string node_name;

  _NewLeaderResponse__isset __isset;

  void __set_node_name(const std::string& val);

  bool operator == (const NewLeaderResponse & rhs) const
  {
    if (!(node_name == rhs.node_name))
      return false;
    return true;
  }
  bool operator != (const NewLeaderResponse &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NewLeaderResponse & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot) override;
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const override;

  virtual void printTo(std::ostream& out) const;
};

void swap(NewLeaderResponse &a, NewLeaderResponse &b);

std::ostream& operator<<(std::ostream& out, const NewLeaderResponse& obj);

} // namespace

#endif
