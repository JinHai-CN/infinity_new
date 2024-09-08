// Copyright(C) 2024 InfiniFlow, Inc. All rights reserved.
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

export module cluster_manager;

import stl;
import third_party;
import status;
import peer_thrift_client;
import peer_task;

namespace infinity {

export class ClusterManager {
public:
    ClusterManager() = default;
    ~ClusterManager();

    Status InitAsLeader(const String& node_name);
    Status InitAsFollower(const String& node_name, const String& leader_ip, i64 leader_port);
    Status InitAsLearner(const String& node_name, const String& leader_ip, i64 leader_port);
    Status UnInit();

    Status Register(SharedPtr<NodeInfo> server_node);
    Status Unregister(const String& node_name); // Used by manual or disconnect from follower/learner

    Status UpdateNodeInfo(const SharedPtr<NodeInfo>& server_node); // Used by leader;
    Status UpdateNonLeaderNodeInfo(const Vector<SharedPtr<NodeInfo>>& info_of_nodes); // Use by follower / learner to update all node info.
    Vector<SharedPtr<NodeInfo>> ListNodes() const;
    SharedPtr<NodeInfo> GetNodeInfoPtrByName(const String& node_name) const;
    SharedPtr<NodeInfo> ThisNode() const;
private:
    mutable std::mutex mutex_;
    Vector<SharedPtr<NodeInfo>> other_nodes_; // Used by leader and follower/learner
    SharedPtr<NodeInfo> this_node_; // Used by leader and follower/learner
    SharedPtr<PeerClient> peer_client_{}; // Used by follower and learner;

    Map<String, SharedPtr<PeerClient>> follower_clients_{}; // Used by leader;
};

}
