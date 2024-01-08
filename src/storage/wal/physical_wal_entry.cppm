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

// namespace infinity {

// export enum class PhysicalWalOperationType : i8 {
//     INVALID = 0,
//     // -----------------------------
//     // Catalog
//     // -----------------------------
//     // CREATE_DATABASE = 1,
//     // DROP_DATABASE = 2,
//     // CREATE_TABLE = 3,
//     // DROP_TABLE = 4,
//     // ALTER_INFO = 5,
//     // CREATE_INDEX = 6,
//     // DROP_INDEX = 7,
//     ADD_DATABASE_ENTRY = 1,
//     DEL_DATABASE_ENTRY = 2,
//
//     ADD_TABLE_ENTRY = 3,
//     DEL_TABLE_ENTRY = 4,
//
//     ADD_SEGMENT_ENTRY = 5,
//     DEL_SEGMRNT_ENTRY = 6,
//
//     ADD_BLOCK_ENTRY = 7,
//     DEL_BLOCK_ENTRY = 8,
//
//     ADD_COLUMN_ENTRY = 9,
//     DEL_COLUMN_ENTRY = 10,
//
//     // -----------------------------
//     // INDEX
//     // -----------------------------
//     ADD_TABLE_INDEX = 11,
//     DEL_TABLE_INDEX = 12,
//
//     ADD_INDEX_ENTRY = 13,
//     DEL_INDEX_ENTRY = 14,
//
//     ADD_COLUMN_INDEX = 14,
//
//     ADD_INDEX_BY_SEGMENT_ID = 15,
//
//     // -----------------------------
//     // Data
//     // -----------------------------
//     // IMPORT = 20,
//     // APPEND = 21,
//     // DELETE = 22,
//     // -----------------------------
//     // Flush
//     // -----------------------------
//     CHECKPOINT = 99,
// };
//
// export class PhysicalWalOperation {
// public:
//     PhysicalWalOperation(u64 txn_id, TxnTimeStamp begin_ts, TxnTimeStamp commit_ts, BooleanT deleted, EntryType entry_type)
//         : txn_id_(txn_id), begin_ts_(begin_ts), commit_ts_(commit_ts), deleted_(deleted), entry_type_(entry_type){};
//     virtual ~PhysicalWalOperation() = default;
//     virtual auto GetType() -> PhysicalWalOperationType;
//     virtual auto operator==(const PhysicalWalOperation &other) const -> bool { return typeid(*this) == typeid(other); }
//     auto operator!=(const PhysicalWalOperation &other) const -> bool { return !(*this == other); }
//     virtual SizeT GetSizeInBytes() const = 0;
//     virtual void WriteAdv(char *&ptr) const = 0;
//
//
//     static SharedPtr<PhysicalWalOperation> ReadAdv(char *&ptr, i32 max_bytes);
//     static String PhysicalWalOperationToString(PhysicalWalOperationType type);
//
// private:
//     u64 txn_id_{0};
//     TxnTimeStamp begin_ts_{0};
//     TxnTimeStamp commit_ts_{0};
//     BooleanT deleted_{false};
//     EntryType entry_type_{EntryType::kDummy};
// };
//
// export class PhysicalWalOperationAddDatabaseEntry : public PhysicalWalOperation {
//     explicit PhysicalWalOperationAddDatabaseEntry(u64 txn_id,
//                                                   TxnTimeStamp begin_ts,
//                                                   TxnTimeStamp commit_ts,
//                                                   BooleanT deleted,
//                                                   EntryType entry_type,
//                                                   String db_name,
//                                                   String data_dir,
//                                                   String db_entry_dir)
//         : PhysicalWalOperation(txn_id, begin_ts, commit_ts, deleted, entry_type), db_name_(Move(db_name)), data_dir_(Move(data_dir)),
//           db_entry_dir_(Move(db_entry_dir)) {}
//     PhysicalWalOperationType GetType() override { return PhysicalWalOperationType::ADD_DATABASE_ENTRY; }
//     auto operator==(const PhysicalWalOperation &other) const -> bool override {
//         const auto *other_cmd = dynamic_cast<const PhysicalWalOperationAddDatabaseEntry *>(&other);
//         return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_);
//     }
//     [[nodiscard]] SizeT GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
// private:
//     // Base db meta
//     String db_name_{};
//     String data_dir_{};
//     // DB entry
//     String db_entry_dir_{};
//     // String db_name_{};
// };
//
// export struct WalCmdDropDatabase : public WalCmd {
//     explicit WalCmdDropDatabase(String db_name) : db_name_(Move(db_name)) {}
//
//     WalCommandType GetType() override { return WalCommandType::DROP_DATABASE; }
//     bool operator==(const WalCmd &other) const override {
//         auto other_cmd = dynamic_cast<const WalCmdDropDatabase *>(&other);
//         return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_);
//     }
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
// };
//
// export struct WalCmdCreateTable : public WalCmd {
//     WalCmdCreateTable(String db_name, const SharedPtr<TableDef> &table_def) : db_name_(Move(db_name)), table_def_(table_def) {}
//
//     WalCommandType GetType() override { return WalCommandType::CREATE_TABLE; }
//     bool operator==(const WalCmd &other) const override;
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     SharedPtr<TableDef> table_def_{};
// };
//
// export struct WalCmdCreateIndex : public WalCmd {
//     WalCmdCreateIndex(String db_name, String table_name, SharedPtr<IndexDef> index_def)
//         : db_name_(Move(db_name)), table_name_(Move(table_name)), index_def_(Move(index_def)) {}
//
//     WalCommandType GetType() override { return WalCommandType::CREATE_INDEX; }
//
//     bool operator==(const WalCmd &other) const override;
//
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     String table_name_{};
//     SharedPtr<IndexDef> index_def_{};
// };
//
// export struct WalCmdDropTable : public WalCmd {
//     WalCmdDropTable(const String &db_name, const String &table_name) : db_name_(db_name), table_name_(table_name) {}
//
//     WalCommandType GetType() override { return WalCommandType::DROP_TABLE; }
//     bool operator==(const WalCmd &other) const override {
//         auto other_cmd = dynamic_cast<const WalCmdDropTable *>(&other);
//         return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_) && IsEqual(table_name_, other_cmd->table_name_);
//     }
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     String table_name_{};
// };
//
// export struct WalCmdDropIndex : public WalCmd {
//     WalCmdDropIndex(const String &db_name, const String &table_name, const String &index_name)
//         : db_name_(db_name), table_name_(table_name), index_name_(index_name) {}
//
//     virtual WalCommandType GetType() override { return WalCommandType::DROP_INDEX; }
//
//     bool operator==(const WalCmd &other) const override;
//
//     i32 GetSizeInBytes() const override;
//
//     void WriteAdv(char *&buf) const override;
//
//     const String db_name_{};
//     const String table_name_{};
//     const String index_name_{};
// };
//
// export struct WalCmdImport : public WalCmd {
//     WalCmdImport(String db_name, String table_name, String segment_dir, u32 segment_id, u16 block_entries_size, Vector<u16> &row_counts)
//         : db_name_(Move(db_name)), table_name_(Move(table_name)), segment_dir_(Move(segment_dir)), segment_id_(segment_id),
//           block_entries_size_(block_entries_size), row_counts_(row_counts) {}
//
//     WalCommandType GetType() override { return WalCommandType::IMPORT; }
//     bool operator==(const WalCmd &other) const override;
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     String table_name_{};
//     String segment_dir_{};
//     u32 segment_id_{};
//     u16 block_entries_size_{};
//     // row_counts_[i] means the number of rows in the i-th block entry
//     Vector<u16> row_counts_{};
// };
//
// export struct WalCmdAppend : public WalCmd {
//     WalCmdAppend(String db_name, String table_name, const SharedPtr<DataBlock> &block)
//         : db_name_(Move(db_name)), table_name_(Move(table_name)), block_(block) {}
//
//     WalCommandType GetType() override { return WalCommandType::APPEND; }
//     bool operator==(const WalCmd &other) const override;
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     String table_name_{};
//     SharedPtr<DataBlock> block_{};
// };
//
// export struct WalCmdDelete : public WalCmd {
//     WalCmdDelete(String db_name, String table_name, const Vector<RowID> &row_ids)
//         : db_name_(Move(db_name)), table_name_(Move(table_name)), row_ids_(row_ids) {}
//
//     WalCommandType GetType() override { return WalCommandType::DELETE; }
//     bool operator==(const WalCmd &other) const override;
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     String table_name_{};
//     Vector<RowID> row_ids_{};
// };
//
// export struct WalCmdCheckpoint : public WalCmd {
//     WalCmdCheckpoint(i64 max_commit_ts, bool is_full_checkpoint, String catalog_path)
//         : max_commit_ts_(max_commit_ts), is_full_checkpoint_(is_full_checkpoint), catalog_path_(catalog_path) {}
//     virtual WalCommandType GetType() override { return WalCommandType::CHECKPOINT; }
//
//     virtual bool operator==(const WalCmd &other) const override;
//
//     virtual i32 GetSizeInBytes() const override;
//
//     void WriteAdv(char *&buf) const override;
//
//     i64 max_commit_ts_{};
//     bool is_full_checkpoint_;
//     String catalog_path_{};
// };
//
// export struct WalEntryHeader {
//     i32 size_{}; // size of payload, including the header, round to multi
//     // of 4. There's 4 bytes pad just after the payload storing
//     // the same value to assist backward iterating.
//     u32 checksum_{}; // crc32 of the entry, including the header and the
//     // payload. User shall populate it before writing to wal.
//     i64 txn_id_{};    // txn id of the entry
//     i64 commit_ts_{}; // commit timestamp of the txn
// };
//
// export struct WalEntry : WalEntryHeader {
//     bool operator==(const WalEntry &other) const;
//
//     bool operator!=(const WalEntry &other) const;
//
//     // Estimated serialized size in bytes, ensured be no less than Write
//     // requires, allowed be larger.
//     [[nodiscard]] i32 GetSizeInBytes() const;
//
//     // Write to a char buffer
//     void WriteAdv(char *&ptr) const;
//     // Read from a serialized version
//     static SharedPtr<WalEntry> ReadAdv(char *&ptr, i32 max_bytes);
//
//     Vector<SharedPtr<WalCmd>> cmds_{};
//
//     [[nodiscard]] Pair<i64, String> GetCheckpointInfo() const;
//
//     [[nodiscard]] bool IsCheckPoint() const;
//
//     [[nodiscard]] bool IsFullCheckPoint() const;
//
//     [[nodiscard]] String ToString() const;
// };
//
// export class WalEntryIterator {
// public:
//     static WalEntryIterator Make(const String &wal_path);
//
//     [[nodiscard]] SharedPtr<WalEntry> Next();
//
// private:
//     WalEntryIterator(Vector<char> &&buf, StreamSize wal_size) : buf_(Move(buf)), wal_size_(wal_size) { end_ = buf_.data() + wal_size_; }
//
//     Vector<char> buf_{};
//     StreamSize wal_size_{};
//     char *end_{};
// };
//
// export class WalListIterator {
// public:
//     explicit WalListIterator(const Vector<String> &wal_list) {
//         for (SizeT i = 0; i < wal_list.size(); ++i) {
//             wal_deque_.push_back(wal_list[i]);
//         }
//     }
//
//     [[nodiscard]] SharedPtr<WalEntry> Next();
//
// private:
//     Deque<String> wal_deque_{};
//     UniquePtr<WalEntryIterator> iter_{};
// };
//
// } // namespace infinity
// // Copyright(C) 2023 InfiniFlow, Inc. All rights reserved.
// //
// // Licensed under the Apache License, Version 2.0 (the "License");
// // you may not use this file except in compliance with the License.
// // You may obtain a copy of the License at
// //
// //     https://www.apache.org/licenses/LICENSE-2.0
// //
// // Unless required by applicable law or agreed to in writing, software
// // distributed under the License is distributed on an "AS IS" BASIS,
// // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// // See the License for the specific language governing permissions and
// // limitations under the License.
//
// module;
//
// #include <typeinfo>
//
// export module wal:physical_wal_entry;
//
// import table_def;
// import index_def;
// import data_block;
// import stl;
// import parser;
// import infinity_exception;
//
// namespace infinity {
//
// export enum class PhysicalWalOperationType : i8 {
//     INVALID = 0,
//     // -----------------------------
//     // Catalog
//     // -----------------------------
//     // CREATE_DATABASE = 1,
//     // DROP_DATABASE = 2,
//     // CREATE_TABLE = 3,
//     // DROP_TABLE = 4,
//     // ALTER_INFO = 5,
//     // CREATE_INDEX = 6,
//     // DROP_INDEX = 7,
//     ADD_DATABASE_ENTRY = 1,
//     DEL_DATABASE_ENTRY = 2,
//
//     ADD_TABLE_ENTRY = 3,
//     DEL_TABLE_ENTRY = 4,
//
//     ADD_SEGMENT_ENTRY = 5,
//     DEL_SEGMRNT_ENTRY = 6,
//
//     ADD_BLOCK_ENTRY = 7,
//     DEL_BLOCK_ENTRY = 8,
//
//     ADD_COLUMN_ENTRY = 9,
//     DEL_COLUMN_ENTRY = 10,
//
//     ADD_TABLE_INDEX = 11,
//     DEL_TABLE_INDEX = 12,
//
//     ADD_INDEX_ENTRY = 13,
//
//     // -----------------------------
//     // Data
//     // -----------------------------
//     IMPORT = 20,
//     APPEND = 21,
//     DELETE = 22,
//     // -----------------------------
//     // Flush
//     // -----------------------------
//     CHECKPOINT = 99,
// };

// // WalCommandType -> String
// export struct WalCmd {
//     virtual ~WalCmd() = default;
//
//     virtual auto GetType() -> WalCommandType = 0;
//
//     virtual auto operator==(const WalCmd &other) const -> bool { return typeid(*this) == typeid(other); }
//     auto operator!=(const WalCmd &other) const -> bool { return !(*this == other); }
//     // Estimated serialized size in bytes
//     [[nodiscard]] virtual i32 GetSizeInBytes() const = 0;
//     // Write to a char buffer
//     virtual void WriteAdv(char *&ptr) const = 0;
//     // Read from a serialized version
//     static SharedPtr<WalCmd> ReadAdv(char *&ptr, i32 max_bytes);
//
//     static String WalCommandTypeToString(WalCommandType type);
// };
//
// export struct WalCmdCreateDatabase : public WalCmd {
//     explicit WalCmdCreateDatabase(String db_name) : db_name_(Move(db_name)) {}
//
//     WalCommandType GetType() override { return WalCommandType::CREATE_DATABASE; }
//     auto operator==(const WalCmd &other) const -> bool override {
//         const auto *other_cmd = dynamic_cast<const WalCmdCreateDatabase *>(&other);
//         return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_);
//     }
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
// };
//
// export struct WalCmdDropDatabase : public WalCmd {
//     explicit WalCmdDropDatabase(String db_name) : db_name_(Move(db_name)) {}
//
//     WalCommandType GetType() override { return WalCommandType::DROP_DATABASE; }
//     bool operator==(const WalCmd &other) const override {
//         auto other_cmd = dynamic_cast<const WalCmdDropDatabase *>(&other);
//         return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_);
//     }
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
// };
//
// export struct WalCmdCreateTable : public WalCmd {
//     WalCmdCreateTable(String db_name, const SharedPtr<TableDef> &table_def) : db_name_(Move(db_name)), table_def_(table_def) {}
//
//     WalCommandType GetType() override { return WalCommandType::CREATE_TABLE; }
//     bool operator==(const WalCmd &other) const override;
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     SharedPtr<TableDef> table_def_{};
// };
//
// export struct WalCmdCreateIndex : public WalCmd {
//     WalCmdCreateIndex(String db_name, String table_name, SharedPtr<IndexDef> index_def)
//         : db_name_(Move(db_name)), table_name_(Move(table_name)), index_def_(Move(index_def)) {}
//
//     WalCommandType GetType() override { return WalCommandType::CREATE_INDEX; }
//
//     bool operator==(const WalCmd &other) const override;
//
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     String table_name_{};
//     SharedPtr<IndexDef> index_def_{};
// };
//
// export struct WalCmdDropTable : public WalCmd {
//     WalCmdDropTable(const String &db_name, const String &table_name) : db_name_(db_name), table_name_(table_name) {}
//
//     WalCommandType GetType() override { return WalCommandType::DROP_TABLE; }
//     bool operator==(const WalCmd &other) const override {
//         auto other_cmd = dynamic_cast<const WalCmdDropTable *>(&other);
//         return other_cmd != nullptr && IsEqual(db_name_, other_cmd->db_name_) && IsEqual(table_name_, other_cmd->table_name_);
//     }
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     String table_name_{};
// };
//
// export struct WalCmdDropIndex : public WalCmd {
//     WalCmdDropIndex(const String &db_name, const String &table_name, const String &index_name)
//         : db_name_(db_name), table_name_(table_name), index_name_(index_name) {}
//
//     virtual WalCommandType GetType() override { return WalCommandType::DROP_INDEX; }
//
//     bool operator==(const WalCmd &other) const override;
//
//     i32 GetSizeInBytes() const override;
//
//     void WriteAdv(char *&buf) const override;
//
//     const String db_name_{};
//     const String table_name_{};
//     const String index_name_{};
// };
//
// export struct WalCmdImport : public WalCmd {
//     WalCmdImport(String db_name, String table_name, String segment_dir, u32 segment_id, u16 block_entries_size, Vector<u16> &row_counts)
//         : db_name_(Move(db_name)), table_name_(Move(table_name)), segment_dir_(Move(segment_dir)), segment_id_(segment_id),
//           block_entries_size_(block_entries_size), row_counts_(row_counts) {}
//
//     WalCommandType GetType() override { return WalCommandType::IMPORT; }
//     bool operator==(const WalCmd &other) const override;
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     String table_name_{};
//     String segment_dir_{};
//     u32 segment_id_{};
//     u16 block_entries_size_{};
//     // row_counts_[i] means the number of rows in the i-th block entry
//     Vector<u16> row_counts_{};
// };
//
// export struct WalCmdAppend : public WalCmd {
//     WalCmdAppend(String db_name, String table_name, const SharedPtr<DataBlock> &block)
//         : db_name_(Move(db_name)), table_name_(Move(table_name)), block_(block) {}
//
//     WalCommandType GetType() override { return WalCommandType::APPEND; }
//     bool operator==(const WalCmd &other) const override;
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     String table_name_{};
//     SharedPtr<DataBlock> block_{};
// };
//
// export struct WalCmdDelete : public WalCmd {
//     WalCmdDelete(String db_name, String table_name, const Vector<RowID> &row_ids)
//         : db_name_(Move(db_name)), table_name_(Move(table_name)), row_ids_(row_ids) {}
//
//     WalCommandType GetType() override { return WalCommandType::DELETE; }
//     bool operator==(const WalCmd &other) const override;
//     [[nodiscard]] i32 GetSizeInBytes() const override;
//     void WriteAdv(char *&buf) const override;
//
//     String db_name_{};
//     String table_name_{};
//     Vector<RowID> row_ids_{};
// };
//
// export struct WalCmdCheckpoint : public WalCmd {
//     WalCmdCheckpoint(i64 max_commit_ts, bool is_full_checkpoint, String catalog_path)
//         : max_commit_ts_(max_commit_ts), is_full_checkpoint_(is_full_checkpoint), catalog_path_(catalog_path) {}
//     virtual WalCommandType GetType() override { return WalCommandType::CHECKPOINT; }
//
//     virtual bool operator==(const WalCmd &other) const override;
//
//     virtual i32 GetSizeInBytes() const override;
//
//     void WriteAdv(char *&buf) const override;
//
//     i64 max_commit_ts_{};
//     bool is_full_checkpoint_;
//     String catalog_path_{};
// };
//
// export struct PhysicalWalEntryHeader {
//     i32 size_{}; // size of payload, including the header, round to multi
//     // of 4. There's 4 bytes pad just after the payload storing
//     // the same value to assist backward iterating.
//     u32 checksum_{}; // crc32 of the entry, including the header and the
//     // payload. User shall populate it before writing to wal.
//     i64 txn_id_{};    // txn id of the entry
//     i64 commit_ts_{}; // commit timestamp of the txn
// };
//
// export struct PhysicalWalEntry : WalEntryHeader {
//     bool operator==(const WalEntry &other) const;
//
//     bool operator!=(const WalEntry &other) const;
//
//     // Estimated serialized size in bytes, ensured be no less than Write
//     // requires, allowed be larger.
//     [[nodiscard]] i32 GetSizeInBytes() const;
//
//     // Write to a char buffer
//     void WriteAdv(char *&ptr) const;
//     // Read from a serialized version
//     static SharedPtr<WalEntry> ReadAdv(char *&ptr, i32 max_bytes);
//
//     Vector<SharedPtr<WalCmd>> cmds_{};
//
//     [[nodiscard]] Pair<i64, String> GetCheckpointInfo() const;
//
//     [[nodiscard]] bool IsCheckPoint() const;
//
//     [[nodiscard]] bool IsFullCheckPoint() const;
//
//     [[nodiscard]] String ToString() const;
// };
//
// // export class WalEntryIterator {
// // public:
// //     static WalEntryIterator Make(const String &wal_path);
// //
// //     [[nodiscard]] SharedPtr<WalEntry> Next();
// //
// // private:
// //     WalEntryIterator(Vector<char> &&buf, StreamSize wal_size) : buf_(Move(buf)), wal_size_(wal_size) { end_ = buf_.data() + wal_size_; }
// //
// //     Vector<char> buf_{};
// //     StreamSize wal_size_{};
// //     char *end_{};
// // };
// //
// // export class WalListIterator {
// // public:
// //     explicit WalListIterator(const Vector<String> &wal_list) {
// //         for (SizeT i = 0; i < wal_list.size(); ++i) {
// //             wal_deque_.push_back(wal_list[i]);
// //         }
// //     }
// //
// //     [[nodiscard]] SharedPtr<WalEntry> Next();
// //
// // private:
// //     Deque<String> wal_deque_{};
// //     UniquePtr<WalEntryIterator> iter_{};
// // };
//
// } // namespace infinity
