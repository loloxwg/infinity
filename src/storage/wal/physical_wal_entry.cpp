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

#include <fstream>

module wal;

import crc;
import serialize;
import data_block;
import table_def;
import index_def;
import infinity_exception;

import stl;
import parser;
import third_party;

namespace infinity {

UniquePtr<PhysicalWalOperation> PhysicalWalOperation::ReadAdv(char *&ptr, i32 max_bytes) {
    char *const ptr_end = ptr + max_bytes;
    UniquePtr<PhysicalWalOperation> operation = nullptr;
    auto operation_type = ReadBufAdv<PhysicalWalOperationType>(ptr);
    switch (operation_type) {
        case PhysicalWalOperationType::ADD_DATABASE_META: {
            String db_name = ReadBufAdv<String>(ptr);
            String data_dir = ReadBufAdv<String>(ptr);
            TxnTimeStamp begin_ts = ReadBufAdv<TxnTimeStamp>(ptr);
            bool is_delete = ReadBufAdv<bool>(ptr);
            operation = MakeUnique<AddDatabaseMetaOperation>(begin_ts, is_delete, db_name, data_dir);
            break;
        }
        case PhysicalWalOperationType::ADD_DATABASE_ENTRY: {
            String db_name = ReadBufAdv<String>(ptr);
            String db_entry_dir = ReadBufAdv<String>(ptr);
            TxnTimeStamp begin_ts = ReadBufAdv<TxnTimeStamp>(ptr);
            bool is_delete = ReadBufAdv<bool>(ptr);
            operation = MakeUnique<AddDatabaseEntryOperation>(begin_ts, is_delete, db_name, db_entry_dir);
            operation->begin_ts_ = begin_ts;
            operation->is_delete_ = is_delete;
            break;
        }
        case PhysicalWalOperationType::ADD_TABLE_ENTRY: {
            String db_name = ReadBufAdv<String>(ptr);
            String table_name = ReadBufAdv<String>(ptr);
            String table_entry_dir = ReadBufAdv<String>(ptr);
            TxnTimeStamp begin_ts = ReadBufAdv<TxnTimeStamp>(ptr);
            bool is_delete = ReadBufAdv<bool>(ptr);
            operation = MakeUnique<AddTableEntryOperation>(begin_ts, is_delete, db_name, table_name, table_entry_dir);
            operation->begin_ts_ = begin_ts;
            operation->is_delete_ = is_delete;
            break;
        }
        case PhysicalWalOperationType::ADD_SEGMENT_ENTRY: {
            String db_name = ReadBufAdv<String>(ptr);
            String table_name = ReadBufAdv<String>(ptr);
            SegmentID segment_id = ReadBufAdv<SegmentID>(ptr);
            String segment_dir = ReadBufAdv<String>(ptr);

            TxnTimeStamp begin_ts = ReadBufAdv<TxnTimeStamp>(ptr);
            bool is_delete = ReadBufAdv<bool>(ptr);
            operation = MakeUnique<AddSegmentEntryOperation>(begin_ts, is_delete, db_name, table_name, segment_id, segment_dir);
            operation->begin_ts_ = begin_ts;
            operation->is_delete_ = is_delete;
            break;
        }
        case PhysicalWalOperationType::ADD_BLOCK_ENTRY: {
            String db_name = ReadBufAdv<String>(ptr);
            String table_name = ReadBufAdv<String>(ptr);
            SegmentID segment_id = ReadBufAdv<SegmentID>(ptr);
            BlockID block_id = ReadBufAdv<BlockID>(ptr);
            String block_dir = ReadBufAdv<String>(ptr);
            u16 row_count = ReadBufAdv<u16>(ptr);
            u16 row_capacity = ReadBufAdv<u16>(ptr);

            TxnTimeStamp begin_ts = ReadBufAdv<TxnTimeStamp>(ptr);
            bool is_delete = ReadBufAdv<bool>(ptr);
            operation = MakeUnique<AddBlockEntryOperation>(begin_ts,
                                                           is_delete,
                                                           db_name,
                                                           table_name,
                                                           segment_id,
                                                           block_id,
                                                           block_dir,
                                                           row_count,
                                                           row_capacity);
            break;
        }
        case PhysicalWalOperationType::ADD_COLUMN_ENTRY: {
            String db_name = ReadBufAdv<String>(ptr);
            String table_name = ReadBufAdv<String>(ptr);
            SegmentID segment_id = ReadBufAdv<SegmentID>(ptr);
            BlockID block_id = ReadBufAdv<BlockID>(ptr);
            ColumnID column_id = ReadBufAdv<ColumnID>(ptr);
            i32 next_outline_idx = ReadBufAdv<i32>(ptr);

            TxnTimeStamp begin_ts = ReadBufAdv<TxnTimeStamp>(ptr);
            bool is_delete = ReadBufAdv<bool>(ptr);

            operation =
                MakeUnique<AddColumnEntryOperation>(begin_ts, is_delete, db_name, table_name, segment_id, block_id, column_id, next_outline_idx);
            break;
        }
        default:
            Error<StorageException>(fmt::format("UNIMPLEMENTED ReadAdv for PhysicalWalOperation type {}", int(operation_type)));
    }

    max_bytes = ptr_end - ptr;
    if (max_bytes < 0) {
        Error<StorageException>("ptr goes out of range when reading PhysicalWalOperation");
    }
    return operation;
}

void AddDatabaseMetaOperation::WriteAdv(char *&buf) const {
    WriteBufAdv(buf, PhysicalWalOperationType::ADD_DATABASE_META);
    WriteBufAdv(buf, this->db_name_);
    WriteBufAdv(buf, this->data_dir_);
    WriteBufAdv(buf, this->begin_ts_);
    WriteBufAdv(buf, this->is_delete_);
}

void AddTableMetaOperation::WriteAdv(char *&buf) const {
    WriteBufAdv(buf, PhysicalWalOperationType::ADD_DATABASE_META);
    WriteBufAdv(buf, this->table_name_);
    WriteBufAdv(buf, this->db_entry_dir_);
    WriteBufAdv(buf, this->begin_ts_);
    WriteBufAdv(buf, this->is_delete_);
}

void AddDatabaseEntryOperation::WriteAdv(char *&buf) const {
    WriteBufAdv(buf, PhysicalWalOperationType::ADD_DATABASE_ENTRY);
    WriteBufAdv(buf, this->db_name_);
    WriteBufAdv(buf, this->db_entry_dir_);

    WriteBufAdv(buf, this->begin_ts_);
    WriteBufAdv(buf, this->is_delete_);
}

void AddTableEntryOperation::WriteAdv(char *&buf) const {
    WriteBufAdv(buf, PhysicalWalOperationType::ADD_TABLE_ENTRY);
    WriteBufAdv(buf, this->db_name_);
    WriteBufAdv(buf, this->table_name_);
    WriteBufAdv(buf, this->table_entry_dir_);

    WriteBufAdv(buf, this->begin_ts_);
    WriteBufAdv(buf, this->is_delete_);
}

void AddSegmentEntryOperation::WriteAdv(char *&buf) const {
    WriteBufAdv(buf, PhysicalWalOperationType::ADD_SEGMENT_ENTRY);
    WriteBufAdv(buf, this->db_name_);
    WriteBufAdv(buf, this->table_name_);
    WriteBufAdv(buf, this->segment_id_);
    WriteBufAdv(buf, this->segment_dir_);

    WriteBufAdv(buf, this->begin_ts_);
    WriteBufAdv(buf, this->is_delete_);
}

void AddBlockEntryOperation ::WriteAdv(char *&buf) const {
    WriteBufAdv(buf, PhysicalWalOperationType::ADD_BLOCK_ENTRY);
    WriteBufAdv(buf, this->db_name_);
    WriteBufAdv(buf, this->table_name_);
    WriteBufAdv(buf, this->segment_id_);
    WriteBufAdv(buf, this->block_id_);
    WriteBufAdv(buf, this->block_dir_);
    WriteBufAdv(buf, this->row_count_);
    WriteBufAdv(buf, this->row_capacity_);

    WriteBufAdv(buf, this->begin_ts_);
    WriteBufAdv(buf, this->is_delete_);
}

void AddColumnEntryOperation::WriteAdv(char *&buf) const {
    WriteBufAdv(buf, PhysicalWalOperationType::ADD_COLUMN_ENTRY);
    WriteBufAdv(buf, this->db_name_);
    WriteBufAdv(buf, this->table_name_);
    WriteBufAdv(buf, this->segment_id_);
    WriteBufAdv(buf, this->block_id_);
    WriteBufAdv(buf, this->column_id_);
    WriteBufAdv(buf, this->next_outline_idx_);

    WriteBufAdv(buf, this->begin_ts_);
    WriteBufAdv(buf, this->is_delete_);
}

/// class PhysicalWalEntry
bool PhysicalWalEntry::operator==(const PhysicalWalEntry &other) const {
    if (this->txn_id_ != other.txn_id_ || this->commit_ts_ != other.commit_ts_ || this->operations_.size() != other.operations_.size()) {
        return false;
    }
    for (u32 i = 0; i < this->operations_.size(); i++) {
        const UniquePtr<PhysicalWalOperation> &operation1 = this->operations_[i];
        const UniquePtr<PhysicalWalOperation> &operation2 = other.operations_[i];
        if (operation1.get() == nullptr || operation2.get() == nullptr || (*operation1).operator!=(*operation2)) {
            return false;
        }
    }
    return true;
}

i32 PhysicalWalEntry::GetSizeInBytes() const {
    i32 size = sizeof(PhysicalWalEntryHeader) + sizeof(i32);
    SizeT operations_size = operations_.size();
    for (SizeT idx = 0; idx < operations_size; ++idx) {
        const auto &operation = operations_[idx];
        size += operation->GetSizeInBytes();
    }
    size += sizeof(i32); // pad
    return size;
}

SharedPtr<PhysicalWalEntry> PhysicalWalEntry::ReadAdv(char *&ptr, i32 max_bytes) {
    char *const ptr_end = ptr + max_bytes;
    if (max_bytes <= 0) {
        Error<StorageException>("ptr goes out of range when reading WalEntry");
    }
    auto entry = MakeShared<PhysicalWalEntry>();
    auto *header = (PhysicalWalEntryHeader *)ptr;
    entry->size_ = header->size_;
    entry->checksum_ = header->checksum_;
    entry->txn_id_ = header->txn_id_;
    entry->commit_ts_ = header->commit_ts_;
    i32 size2 = *(i32 *)(ptr + entry->size_ - sizeof(i32));
    if (entry->size_ != size2) {
        return nullptr;
    }
    header->checksum_ = 0;
    u32 checksum2 = CRC32IEEE::makeCRC(reinterpret_cast<const unsigned char *>(ptr), entry->size_);
    if (entry->checksum_ != checksum2) {
        return nullptr;
    }
    ptr += sizeof(PhysicalWalEntryHeader);
    i32 cnt = ReadBufAdv<i32>(ptr);
    for (i32 i = 0; i < cnt; i++) {
        max_bytes = ptr_end - ptr;
        if (max_bytes <= 0) {
            Error<StorageException>("ptr goes out of range when reading WalEntry");
        }
        UniquePtr<PhysicalWalOperation> operation = PhysicalWalOperation::ReadAdv(ptr, max_bytes);
        entry->operations_.emplace_back(std::move(operation));
    }
    ptr += sizeof(i32);
    max_bytes = ptr_end - ptr;
    if (max_bytes < 0) {
        Error<StorageException>("ptr goes out of range when reading WalEntry");
    }
    return entry;
}

/**
 * An entry is serialized as follows:
 * - PhysicalWalEntryHeader
 *   - size
 *   - checksum
 *   - txn_id
 *   - commit_ts
 * - number of WalCmd
 *   - (repeated) WalCmd
 * - 4 bytes pad
 * @param ptr
 * @return void
 */
void PhysicalWalEntry::WriteAdv(char *&ptr) const {
    char *const saved_ptr = ptr;
    std::memcpy(ptr, this, sizeof(PhysicalWalEntryHeader));
    ptr += sizeof(PhysicalWalEntryHeader);

    WriteBufAdv(ptr, static_cast<i32>(operations_.size()));
    SizeT operation_count = operations_.size();
    for (SizeT idx = 0; idx < operation_count; ++idx) {
        const auto &operation = operations_[idx];
        operation->WriteAdv(ptr);
    }
    i32 size = ptr - saved_ptr + sizeof(i32);
    WriteBufAdv(ptr, size);
    auto *header = (PhysicalWalEntryHeader *)saved_ptr;
    header->size_ = size;
    header->checksum_ = 0;
    // CRC32IEEE is equivalent to boost::crc_32_type on
    // little-endian machine.
    header->checksum_ = CRC32IEEE::makeCRC(reinterpret_cast<const unsigned char *>(saved_ptr), size);
}
} // namespace infinity
