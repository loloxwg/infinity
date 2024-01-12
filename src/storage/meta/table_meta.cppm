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

export module catalog:table_meta;

import :base_entry;

import stl;
import parser;
import third_party;
import table_entry_type;
import buffer_manager;
import status;

namespace infinity {

export class DBEntry;
class TableEntry;
class TxnManager;

export struct TableMeta {

    friend class DBEntry;
    friend struct NewCatalog;

public:
    inline explicit TableMeta(const SharedPtr<String> &db_entry_dir, SharedPtr<String> table_name, DBEntry *db_entry)
        : db_entry_dir_(db_entry_dir), table_name_(std::move(table_name)), db_entry_(db_entry) {}

    static UniquePtr<TableMeta> NewTableMeta(const SharedPtr<String> &db_entry_dir,
                                             SharedPtr<String> name,
                                             DBEntry *db_entry,
                                             TxnManager *txn_mgr,
                                             TransactionID txn_id,
                                             TxnTimeStamp begin_ts,
                                             bool is_delete = false);

    SharedPtr<String> ToString();

    nlohmann::json Serialize(TxnTimeStamp max_commit_ts, bool is_full_checkpoint);

    static UniquePtr<TableMeta> Deserialize(const nlohmann::json &table_meta_json, DBEntry *db_entry, BufferManager *buffer_mgr);

    void MergeFrom(TableMeta &other);

    [[nodiscard]] const SharedPtr<String> &table_name_ptr() const { return table_name_; }
    [[nodiscard]] const String &table_name() const { return *table_name_; }
    const SharedPtr<String> &db_name_ptr() const;
    const String &db_name() const;

private:
    Tuple<TableEntry *, Status> CreateNewEntry(TableEntryType table_entry_type,
                                               const SharedPtr<String> &table_collection_name,
                                               const Vector<SharedPtr<ColumnDef>> &columns,
                                               u64 txn_id,
                                               TxnTimeStamp begin_ts,
                                               TxnManager *txn_mgr);

    Tuple<TableEntry *, Status>
    DropNewEntry(u64 txn_id, TxnTimeStamp begin_ts, TxnManager *txn_mgr, const String &table_name, ConflictType conflict_type);

    void DeleteNewEntry(u64 txn_id, TxnManager *txn_mgr);

    Tuple<TableEntry *, Status> GetEntry(u64 txn_id, TxnTimeStamp begin_ts);

private:
    std::shared_mutex rw_locker_{};
    SharedPtr<String> db_entry_dir_{};
    SharedPtr<String> table_name_{};

    DBEntry *db_entry_{};

    // Ordered by commit_ts from latest to oldest.
    List<UniquePtr<BaseEntry>> entry_list_{};
};

} // namespace infinity
