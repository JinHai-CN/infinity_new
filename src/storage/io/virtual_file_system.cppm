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

export module virtual_file_system;

import stl;
import status;
import third_party;
import virtual_file_system_type;
import abstract_file_handle;

namespace infinity {

export class LocalDiskCache {
public:
    LocalDiskCache(u64 disk_capacity_limit, SharedPtr<String> cache_dir, SizeT lru_count) : cache_dir_(std::move(cache_dir)) {}

    ~LocalDiskCache() = default;

private:
    SharedPtr<String> cache_dir_{};
};

export class VirtualFileSystem {
    // Factory class to generate different type of file handle
public:
    Status Init(FSType fs_type, Map<String, String> &config);
    Status UnInit();
    UniquePtr<AbstractFileHandle> BuildFileHandle(const String &path, FileAccessMode access_mode) const;
    LocalDiskCache *GetLocalDiskCache() const;

private:
    FSType fs_type_{FSType::kLocal};
    UniquePtr<LocalDiskCache> local_disk_cache_{};

    // Using by minio
    UniquePtr<minio::s3::BaseUrl> minio_base_url_{};
    UniquePtr<minio::creds::StaticProvider> minio_provider_{};
    UniquePtr<minio::s3::Client> minio_client_{};
};

} // namespace infinity
