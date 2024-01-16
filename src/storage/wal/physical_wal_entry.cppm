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

#include <parallel_hashmap/phmap_utils.h>
#include <typeinfo>

export module wal:physical_wal_entry;

import table_def;
import index_def;
import data_block;
import stl;
import parser;
import infinity_exception;
import catalog;
import outline_info;

namespace infinity {

export enum class PhysicalWalOperationType : i8 {
    INVALID = 0,
    // -----------------------------
    // Meta
    // -----------------------------
    ADD_DATABASE_META = 1,
    ADD_TABLE_META = 2,
    // -----------------------------
    // Entry
    // -----------------------------
    ADD_DATABASE_ENTRY = 11,
    ADD_TABLE_ENTRY = 12,
    ADD_SEGMENT_ENTRY = 13,
    ADD_BLOCK_ENTRY = 14,
    ADD_COLUMN_ENTRY = 15,

    // -----------------------------
    // INDEX
    // -----------------------------
    ADD_TABLE_INDEX = 21,
    ADD_INDEX_ENTRY = 22,
    ADD_COLUMN_INDEX = 23,
    ADD_INDEX_BY_SEGMENT_ID = 24,
};

/// class PhysicalWalOperation
export class PhysicalWalOperation {
public:
    PhysicalWalOperation() = default;
    PhysicalWalOperation(PhysicalWalOperationType type) : type_(type) {}
    PhysicalWalOperation(TxnTimeStamp begin_ts, bool is_delete) : begin_ts_(begin_ts), is_delete_(is_delete) {}
    virtual ~PhysicalWalOperation() = default;
    virtual auto GetType() -> PhysicalWalOperationType = 0; // This is a pure virtual function
    virtual auto operator==(const PhysicalWalOperation &other) const -> bool { return typeid(*this) == typeid(other); }
    auto operator!=(const PhysicalWalOperation &other) const -> bool { return !(*this == other); }
    [[nodiscard]] virtual SizeT GetSizeInBytes() const = 0; // This is a pure virtual function
    virtual void WriteAdv(char *&ptr) const = 0;
    static UniquePtr<PhysicalWalOperation> ReadAdv(char *&ptr, i32 max_bytes);
    SizeT GetBaseSizeInBytes() const { return sizeof(TxnTimeStamp) + sizeof(bool); }
    virtual void Snapshot() = 0;
    virtual const String ToString() const = 0;

public:
    TxnTimeStamp begin_ts_{0};
    bool is_delete_{false};
    bool is_flushed_{false};
    bool is_snapshoted_{false};
    PhysicalWalOperationType type_{};
};

/// class AddDatabaseMetaOperation
export class AddDatabaseMetaOperation : public PhysicalWalOperation {
public:
    explicit AddDatabaseMetaOperation(TxnTimeStamp begin_ts, bool is_delete, String db_name, String data_dir)
        : PhysicalWalOperation(begin_ts, is_delete), db_name_(std::move(db_name)), data_dir_(std::move(data_dir)) {}
    explicit AddDatabaseMetaOperation(DBMeta *db_meta) : PhysicalWalOperation(PhysicalWalOperationType::ADD_DATABASE_META), db_meta_(db_meta) {}
    PhysicalWalOperationType GetType() override { return PhysicalWalOperationType::ADD_DATABASE_META; }
    auto operator==(const PhysicalWalOperation &other) const -> bool override {
        const auto *other_cmd = dynamic_cast<const AddDatabaseMetaOperation *>(&other);
        return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_);
    }
    [[nodiscard]] SizeT GetSizeInBytes() const override {
        auto total_size =
            sizeof(PhysicalWalOperationType) + sizeof(i32) + this->db_name_.size() + sizeof(i32) + this->data_dir_.size() + GetBaseSizeInBytes();
        return total_size;
    }
    void WriteAdv(char *&buf) const override;
    void Snapshot() override;
    const String ToString() const override { return String("AddDatabaseMetaOperation"); }

public:
    DBMeta *db_meta_{};

private:
    String db_name_{};
    String data_dir_{};
};

/// class AddTableMetaOperation
export class AddTableMetaOperation : public PhysicalWalOperation {
public:
    explicit AddTableMetaOperation(TxnTimeStamp begin_ts, bool is_delete, String table_name, String db_entry_dir_)
        : PhysicalWalOperation(begin_ts, is_delete), table_name_(std::move(table_name)), db_entry_dir_(std::move(db_entry_dir_)) {}
    explicit AddTableMetaOperation(TableMeta *table_meta) : PhysicalWalOperation(PhysicalWalOperationType::ADD_TABLE_META), table_meta_(table_meta) {}
    PhysicalWalOperationType GetType() override { return PhysicalWalOperationType::ADD_DATABASE_META; }
    auto operator==(const PhysicalWalOperation &other) const -> bool override {
        const auto *other_cmd = dynamic_cast<const AddTableMetaOperation *>(&other);
        return other_cmd != nullptr && IsEqual(table_name_, other_cmd->table_name_);
    }
    [[nodiscard]] SizeT GetSizeInBytes() const override {
        auto total_size = sizeof(PhysicalWalOperationType) + sizeof(i32) + this->table_name_.size() + sizeof(i32) + this->db_entry_dir_.size() +
                          GetBaseSizeInBytes();
        return total_size;
    }
    void WriteAdv(char *&buf) const override;
    void Snapshot() override;
    const String ToString() const override { return "AddTableMetaOperation"; }

public:
    TableMeta *table_meta_{};

private:
    String table_name_{};
    String db_entry_dir_{};
};

/// class AddDatabaseEntryOperation
export class AddDatabaseEntryOperation : public PhysicalWalOperation {
public:
    explicit AddDatabaseEntryOperation(TxnTimeStamp begin_ts, bool is_delete, String db_name, String db_entry_dir)
        : PhysicalWalOperation(begin_ts, is_delete), db_name_(std::move(db_name)), db_entry_dir_(std::move(db_entry_dir)) {}
    explicit AddDatabaseEntryOperation(SharedPtr<DBEntry> db_entry)
        : PhysicalWalOperation(PhysicalWalOperationType::ADD_DATABASE_ENTRY), db_entry_(db_entry) {}
    PhysicalWalOperationType GetType() override { return PhysicalWalOperationType::ADD_DATABASE_ENTRY; }
    auto operator==(const PhysicalWalOperation &other) const -> bool override {
        const auto *other_cmd = dynamic_cast<const AddDatabaseEntryOperation *>(&other);
        return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_);
    }
    [[nodiscard]] SizeT GetSizeInBytes() const override {
        auto total_size = sizeof(PhysicalWalOperationType) + sizeof(i32) + this->db_name_.size() + sizeof(i32) + this->db_entry_dir_.size() +
                          sizeof(bool) + GetBaseSizeInBytes();
        return total_size;
    }
    void WriteAdv(char *&buf) const override;
    void Snapshot() override;
    const String ToString() const override { return "AddDatabaseEntryOperation"; }

public:
    SharedPtr<DBEntry> db_entry_{};

private:
    String db_name_{};
    String db_entry_dir_{};
};

/// class AddTableEntryOperation
export class AddTableEntryOperation : public PhysicalWalOperation {
public:
    explicit AddTableEntryOperation(TxnTimeStamp begin_ts, bool is_delete, String db_name, String table_name, String table_entry_dir)
        : PhysicalWalOperation(begin_ts, is_delete), db_name_(std::move(db_name)), table_name_(std::move(table_name)),
          table_entry_dir_(std::move(table_entry_dir)) {}
    explicit AddTableEntryOperation(SharedPtr<TableEntry> table_entry)
        : PhysicalWalOperation(PhysicalWalOperationType::ADD_TABLE_ENTRY), table_entry_(table_entry) {}
    PhysicalWalOperationType GetType() override { return PhysicalWalOperationType::ADD_TABLE_ENTRY; }
    auto operator==(const PhysicalWalOperation &other) const -> bool override {
        const auto *other_cmd = dynamic_cast<const AddTableEntryOperation *>(&other);
        return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_) && IsEqual(table_name_, other_cmd->table_name_);
    }
    [[nodiscard]] SizeT GetSizeInBytes() const override {
        auto total_size = sizeof(PhysicalWalOperationType) + sizeof(i32) + this->db_name_.size() + sizeof(i32) + this->table_name_.size() +
                          sizeof(i32) + this->table_entry_dir_.size() + GetBaseSizeInBytes();
        return total_size;
    }
    void WriteAdv(char *&buf) const override;
    void Snapshot() override;
    const String ToString() const override { return "AddTableEntryOperation"; }

public:
    SharedPtr<TableEntry> table_entry_{};

private:
    String db_name_{};
    String table_name_{};
    String table_entry_dir_{};
};

/// class AddSegmentEntryOperation
export class AddSegmentEntryOperation : public PhysicalWalOperation {
public:
    explicit AddSegmentEntryOperation(TxnTimeStamp begin_ts,
                                      bool is_delete,
                                      String db_name,
                                      String table_name,
                                      SegmentID segment_id,
                                      String segment_dir)
        : PhysicalWalOperation(begin_ts, is_delete), db_name_(std::move(db_name)), table_name_(std::move(table_name)), segment_id_(segment_id),
          segment_dir_(std::move(segment_dir)) {}
    explicit AddSegmentEntryOperation(SegmentEntry *segment_entry)
        : PhysicalWalOperation(PhysicalWalOperationType::ADD_SEGMENT_ENTRY), segment_entry_(segment_entry) {}
    PhysicalWalOperationType GetType() override { return PhysicalWalOperationType::ADD_SEGMENT_ENTRY; }
    auto operator==(const PhysicalWalOperation &other) const -> bool override {
        const auto *other_cmd = dynamic_cast<const AddSegmentEntryOperation *>(&other);
        return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_) && IsEqual(table_name_, other_cmd->table_name_) &&
               IsEqual(segment_dir_, other_cmd->segment_dir_) && segment_id_ == other_cmd->segment_id_;
    }
    [[nodiscard]] SizeT GetSizeInBytes() const override {
        auto total_size = sizeof(PhysicalWalOperationType) + sizeof(i32) + this->db_name_.size() + sizeof(i32) + this->table_name_.size() +
                          sizeof(i32) + this->segment_dir_.size() + sizeof(u32) + GetBaseSizeInBytes();
        return total_size;
    }
    void WriteAdv(char *&buf) const override;
    void Snapshot() override;
    const String ToString() const override { return "AddSegmentEntryOperation"; }

public:
    SegmentEntry *segment_entry_{};

private:
    String db_name_{};
    String table_name_{};
    SegmentID segment_id_{};
    String segment_dir_{};
};

/// class AddBlockEntryOperation
export class AddBlockEntryOperation : public PhysicalWalOperation {
public:
    // For create
    AddBlockEntryOperation(TxnTimeStamp begin_ts,
                           bool is_delete,
                           String db_name,
                           String table_name,
                           SegmentID segment_id,
                           BlockID block_id,
                           String block_dir)
        : PhysicalWalOperation(begin_ts, is_delete), db_name_(std::move(db_name)), table_name_(std::move(table_name)), segment_id_(segment_id),
          block_id_(block_id), block_dir_(std::move(block_dir)) {}
    // For update
    AddBlockEntryOperation(TxnTimeStamp begin_ts,
                           bool is_delete,
                           String db_name,
                           String table_name,
                           SegmentID segment_id,
                           BlockID block_id,
                           String block_dir,
                           u16 row_count,
                           u16 row_capacity)
        : PhysicalWalOperation(begin_ts, is_delete), db_name_(std::move(db_name)), table_name_(std::move(table_name)), segment_id_(segment_id),
          block_id_(block_id), block_dir_(std::move(block_dir)), row_count_(row_count), row_capacity_(row_capacity) {}
    explicit AddBlockEntryOperation(BlockEntry *block_entry)
        : PhysicalWalOperation(PhysicalWalOperationType::ADD_BLOCK_ENTRY), block_entry_(block_entry) {}
    PhysicalWalOperationType GetType() override { return PhysicalWalOperationType::ADD_BLOCK_ENTRY; }
    auto operator==(const PhysicalWalOperation &other) const -> bool override {
        const auto *other_cmd = dynamic_cast<const AddBlockEntryOperation *>(&other);
        return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_) && IsEqual(table_name_, other_cmd->table_name_) &&
               segment_id_ == other_cmd->segment_id_ && block_id_ == other_cmd->block_id_;
    }
    [[nodiscard]] SizeT GetSizeInBytes() const override {
        auto total_size = sizeof(PhysicalWalOperationType) + sizeof(i32) + this->db_name_.size() + sizeof(i32) + this->table_name_.size() +
                          +sizeof(SegmentID) + sizeof(BlockID) + sizeof(i32) + this->block_dir_.size() + GetBaseSizeInBytes();
        return total_size;
    }
    void WriteAdv(char *&buf) const override;
    void Snapshot() override;
    const String ToString() const override { return "AddBlockEntryOperation"; }

public:
    BlockEntry *block_entry_{};

private:
    String db_name_{};
    String table_name_{};
    SegmentID segment_id_{0};
    BlockID block_id_{0};
    String block_dir_{};

private:
    // For update
    u16 row_count_{0};
    u16 row_capacity_{0};
};

/// class AddColumnEntryOperation
export class AddColumnEntryOperation : public PhysicalWalOperation {
public:
    // For create
    explicit AddColumnEntryOperation(TxnTimeStamp begin_ts,
                                     bool is_delete,
                                     String db_name,
                                     String table_name,
                                     u32 segment_id,
                                     u16 block_id,
                                     u64 column_id)
        : PhysicalWalOperation(begin_ts, is_delete), db_name_(std::move(db_name)), table_name_(std::move(table_name)), segment_id_(segment_id),
          block_id_(block_id), column_id_(column_id) {}
    // For update
    explicit AddColumnEntryOperation(TxnTimeStamp begin_ts,
                                     bool is_delete,
                                     String db_name,
                                     String table_name,
                                     u32 segment_id,
                                     u16 block_id,
                                     u64 column_id,
                                     i32 next_line_idx)
        : PhysicalWalOperation(begin_ts, is_delete), db_name_(std::move(db_name)), table_name_(std::move(table_name)), segment_id_(segment_id),
          block_id_(block_id), column_id_(column_id), next_outline_idx_(next_line_idx) {}
    explicit AddColumnEntryOperation(BlockColumnEntry *column_entry)
        : PhysicalWalOperation(PhysicalWalOperationType::ADD_COLUMN_ENTRY), column_entry_(column_entry) {}

    PhysicalWalOperationType GetType() override { return PhysicalWalOperationType::ADD_COLUMN_ENTRY; }
    auto operator==(const PhysicalWalOperation &other) const -> bool override {
        const auto *other_cmd = dynamic_cast<const AddColumnEntryOperation *>(&other);
        return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_) && IsEqual(table_name_, other_cmd->table_name_) &&
               segment_id_ == other_cmd->segment_id_ && block_id_ == other_cmd->block_id_ && column_id_ == other_cmd->column_id_ &&
               next_outline_idx_ == other_cmd->next_outline_idx_;
    }
    [[nodiscard]] SizeT GetSizeInBytes() const override {
        auto total_size = sizeof(PhysicalWalOperationType) + sizeof(i32) + this->db_name_.size() + sizeof(i32) + this->table_name_.size() +
                          +sizeof(SegmentID) + sizeof(BlockID) + sizeof(ColumnID) + sizeof(i32) + GetBaseSizeInBytes();
        return total_size;
    }
    void WriteAdv(char *&buf) const override;
    void Snapshot() override;
    const String ToString() const override { return "AddColumnEntryOperation"; }

public:
    BlockColumnEntry *column_entry_{};

private:
    String db_name_{};
    String table_name_{};
    SegmentID segment_id_{};
    BlockID block_id_{};
    ColumnID column_id_{};
    i32 next_outline_idx_{-1}; // -1 for not having outline info
};

/// class PhysicalWalEntryHeader
export class PhysicalWalEntryHeader {
public:
    i32 size_{}; // size of payload, including the header, round to multi
    // of 4. There's 4 bytes pad just after the payload storing
    // the same value to assist backward iterating.
    u32 checksum_{}; // crc32 of the entry, including the header and the
    // payload. User shall populate it before writing to wal.
    TransactionID txn_id_{};   // txn id of the entry
    TxnTimeStamp commit_ts_{}; // commit timestamp of the txn
    // TODO maybe add checkpoint ts for class member
};

/// class PhysicalWalEntry
export class PhysicalWalEntry : PhysicalWalEntryHeader {
public:
    bool operator==(const PhysicalWalEntry &other) const;
    bool operator!=(const PhysicalWalEntry &other) const { return !operator==(other); }
    [[nodiscard]] i32 GetSizeInBytes() const;
    void WriteAdv(char *&ptr) const;
    static SharedPtr<PhysicalWalEntry> ReadAdv(char *&ptr, i32 max_bytes);
    [[nodiscard]] String ToString() const;
    void Snapshot(TransactionID txn_id, TxnTimeStamp commit_ts);

    Vector<UniquePtr<PhysicalWalOperation>> &operations() { return operations_; }

private:
    Vector<UniquePtr<PhysicalWalOperation>> operations_{};
};

} // namespace infinity
