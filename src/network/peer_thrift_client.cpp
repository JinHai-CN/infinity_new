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

module peer_thrift_client;

import stl;
import third_party;
import logger;
import peer_server_thrift_types;
import status;
import thrift;
import infinity_exception;
import peer_task;

namespace infinity {

PeerClient::~PeerClient() {
    if(running_) {
        UnInit();
    }
}

Status PeerClient::Init() {

    Status status = Status::OK();
    try {
        socket_ = MakeShared<TSocket>(node_info_.ip_address_, node_info_.port_);

        TSocket* socket = static_cast<TSocket*>(socket_.get());
        socket->setConnTimeout(2000); // 2s to timeout
        transport_ = MakeShared<TBufferedTransport>(socket_);
        protocol_ = MakeShared<TBinaryProtocol>(transport_);
        client_ = MakeUnique<PeerServiceClient>(protocol_);
        transport_->open();
//        processor_thread_ = MakeShared<Thread>([this] { LOG_INFO(fmt::format("ip: {}", this->node_info_.ip_address_)); });
        processor_thread_ = MakeShared<Thread>([this] { this->Process(); });
//        processor_thread_->detach();
    } catch (const std::exception& e) {
        status = Status::CantConnectServer(node_info_.ip_address_, node_info_.port_, e.what());
    }

    return status;
}

/// TODO: comment
Status PeerClient::UnInit() {
    LOG_INFO(fmt::format("Peer client: {} is stopping.", node_info_.node_name_));
    if(processor_thread_.get() != nullptr) {
        SharedPtr<TerminatePeerTask> terminate_task = MakeShared<TerminatePeerTask>();
        peer_task_queue_.Enqueue(terminate_task);
        terminate_task->Wait();
        processor_thread_->join();
    }
    LOG_INFO(fmt::format("Peer client: {} is stopped.", node_info_.node_name_));
    return Status::OK();
}

void PeerClient::Send(SharedPtr<PeerTask> peer_task) {
    ++peer_task_count_;
    peer_task_queue_.Enqueue(std::move(peer_task));
}

void PeerClient::Process() {
    Deque<SharedPtr<PeerTask>> peer_tasks;
    running_ = true;
    while (running_) {
        peer_task_queue_.DequeueBulk(peer_tasks);
        for (const auto &peer_task : peer_tasks) {
            switch (peer_task->Type()) {
                case PeerTaskType::kTerminate: {
                    LOG_INFO("Stop the background processor");
                    running_ = false;
                }
                case PeerTaskType::kRegister: {
                    LOG_DEBUG(peer_task->ToString());
                    RegisterPeerTask* register_peer_task = static_cast<RegisterPeerTask*>(peer_task.get());
                    Register(register_peer_task);
                    break;
                }
                case PeerTaskType::kUnregister: {
                    LOG_INFO(peer_task->ToString());
                    break;
                }
                case PeerTaskType::kHeartBeat: {
                    LOG_INFO(peer_task->ToString());
                    break;
                }
                default: {
                    String error_message = fmt::format("Invalid peer task type");
                    UnrecoverableError(error_message);
                    break;
                }
            }
            peer_task->Complete();
        }
        peer_task_count_ -= peer_tasks.size();
        peer_tasks.clear();
    }
}

void PeerClient::Register(RegisterPeerTask* peer_task) {

    RegisterRequest request;
    request.node_name = peer_task->node_name_;
    switch(peer_task->node_role_) {
        case NodeRole::kFollower: {
            request.node_type = NodeType::type::kFollower;
            break;
        }
        case NodeRole::kLearner: {
            request.node_type = NodeType::type::kLearner;
            break;
        }
        default: {
            String error_message = fmt::format("Only follower and learner can send register message to leader");
            UnrecoverableError(error_message);
        }
    }
    request.node_ip = peer_task->node_ip_;
    request.node_port = peer_task->node_port_;
    request.txn_timestamp = peer_task->txn_ts_;
    request.message_time = peer_task->message_time_;

    RegisterResponse response;
    client_->Register(response, request);
    if(response.error_code != 0) {
        // Error
        peer_task->error_code_ = response.error_code;
        peer_task->error_message_ = response.error_msg;
    } else {
        peer_task->leader_name_ = response.leader_name;
        peer_task->leader_term_ = response.leader_term;
        peer_task->heartbeat_interval_ = response.heart_beat_interval;
        peer_task->leader_update_time_ = response.message_time;
    }
}

} // namespace infinity
