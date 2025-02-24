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

import global_resource_usage;
import stl;
import vector_heap_chunk;
import default_values;
import parser;

export module fix_heap;

namespace infinity {

class BufferManager;
class BlockColumnEntry;
class VarcharNextCharIterator;

export struct FixHeapManager {
    // Use to store string.
    static constexpr u64 INVALID_CHUNK_OFFSET = std::numeric_limits<u64>::max();

public:
    explicit FixHeapManager(u64 chunk_size = DEFAULT_FIXLEN_CHUNK_SIZE);

    explicit FixHeapManager(BufferManager *buffer_mgr, BlockColumnEntry *block_column_entry, u64 chunk_size = DEFAULT_FIXLEN_CHUNK_SIZE);

    ~FixHeapManager();

    // return value: start chunk id & chunk offset
    Pair<ChunkId, u64> AppendToHeap(const char *data_ptr, SizeT nbytes);

    // return value: start chunk id & chunk offset
    Pair<ChunkId, u64> AppendToHeap(FixHeapManager *src_heap_mgr, ChunkId src_chunk_id, u64 src_chunk_offset, SizeT nbytes);

    // Read #nbytes size of data from offset: #chunk_offset of chunk: #chunk_id to buffer: #buffer, Make sure the buffer has enough space to hold
    // the size of data.
    void ReadFromHeap(char *buffer, ChunkId chunk_id, u64 chunk_offset, SizeT nbytes);

    [[nodiscard]] String Stats() const;

public:
    // Shouldn't use it except unit test
    [[nodiscard]] inline SizeT chunk_count() const { return chunks_.size(); }

    [[nodiscard]] inline ChunkId current_chunk_idx() const { return current_chunk_idx_; }

    [[nodiscard]] inline u64 current_chunk_size() const { return current_chunk_size_; }

    [[nodiscard]] inline u64 total_size() const { return current_chunk_size_ * current_chunk_idx_ + current_chunk_offset_; }

    [[nodiscard]] inline u64 total_mem() const { return current_chunk_size_ * (current_chunk_idx_ + 1); }

public:
    // Used when comparing two VarcharT variables.
    // Only get next char, without copying it to buffer.
    friend VarcharNextCharIterator;
    [[nodiscard]] VarcharNextCharIterator GetNextCharIterator(const VarcharT &varchar);

private:
    VectorHeapChunk AllocateChunk();

    // Allocate new chunk if current chunk is not enough.
    // return value: start chunk id and start offset of the chunk id
    Pair<ChunkId, u64> Allocate(SizeT nbytes);

    // Load chunk from disk if `buffer_mgr_ != nullptr` and that chunk not in memory
    VectorHeapChunk &ReadChunk(ChunkId chunk_id);

private:
    HashMap<ChunkId, VectorHeapChunk> chunks_{};
    u64 current_chunk_size_{DEFAULT_FIXLEN_CHUNK_SIZE};
    ChunkId current_chunk_idx_{0};
    u64 current_chunk_offset_{0};

    BufferManager *buffer_mgr_{nullptr};

    BlockColumnEntry *block_column_entry_{nullptr};
};

// can only move forward
class VarcharNextCharIterator {
public:
    explicit VarcharNextCharIterator(FixHeapManager *heap_mgr, const VarcharT &varchar);

    [[nodiscard]] char GetNextChar();

private:
    const char *data_ptr_{nullptr};
    u64 remain_size_{0};
    FixHeapManager *heap_mgr_{nullptr};
    ChunkId chunk_id_{0};
};

} // namespace infinity
