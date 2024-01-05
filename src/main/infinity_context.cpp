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

module infinity_context;

import stl;
import logger;
import config;
import resource_manager;
import task_scheduler;
import storage;
import session_manager;

namespace infinity {

void InfinityContext::Init(const std::shared_ptr<std::string> &config_path) {
    if (initialized_) {
        return;
    } else {
        // Config
        config_ = std::make_unique<Config>();
        config_->Init(config_path);

        Logger::Initialize(config_.get());

        resource_manager_ = std::make_unique<ResourceManager>(config_->worker_cpu_limit(), config_->total_memory_size());

        task_scheduler_ = std::make_unique<TaskScheduler>(config_.get());

        session_mgr_ = std::make_unique<SessionManager>();

        storage_ = std::make_unique<Storage>(config_.get());
        storage_->Init();

        initialized_ = true;
    }
}

void InfinityContext::UnInit() {
    if (!initialized_) {
        return;
    }
    initialized_ = false;

    storage_->UnInit();
    storage_.reset();

    session_mgr_.reset();

    task_scheduler_->UnInit();
    task_scheduler_.reset();

    resource_manager_.reset();

    Logger::Shutdown();

    config_.reset();
}

} // namespace infinity
