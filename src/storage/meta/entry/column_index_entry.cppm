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

export module catalog:column_index_entry;

import :base_entry;
import :segment_column_index_entry;

import stl;
import parser;
import index_base;
import third_party;
import index_base;
import index_file_worker;
import status;

namespace infinity {

export struct TableIndexEntry;

class BufferManager;
struct TableEntry;
class Txn;

struct ColumnIndexEntry : public BaseEntry {
    friend struct TableEntry;
    friend struct TableIndexEntry;

public:
    ColumnIndexEntry(SharedPtr<IndexBase> index_base,
                     TableIndexEntry *table_index_entry,
                     ColumnID column_id,
                     SharedPtr<String> index_dir,
                     TransactionID txn_id,
                     TxnTimeStamp begin_ts);

    static SharedPtr<ColumnIndexEntry> NewColumnIndexEntry(SharedPtr<IndexBase> index_base,
                                                           u64 column_id,
                                                           TableIndexEntry *table_index_entry,
                                                           Txn* txn,
                                                           TransactionID txn_id,
                                                           SharedPtr<String> col_index_dir,
                                                           TxnTimeStamp begin_ts);

    nlohmann::json Serialize(TxnTimeStamp max_commit_ts);

    static SharedPtr<ColumnIndexEntry>
    Deserialize(const nlohmann::json &column_index_entry_json, TableIndexEntry *table_index_entry, BufferManager *buffer_mgr, TableEntry *table_entry);

public:
    // Getter
    const SharedPtr<String> &col_index_dir() const { return col_index_dir_; }
    ColumnID column_id() const { return column_id_; }
    const IndexBase *index_base_ptr() const { return index_base_.get(); }
    TableIndexEntry *table_index_entry() const { return table_index_entry_; }
    const HashMap<u32, SharedPtr<SegmentColumnIndexEntry>>& index_by_segment() const { return index_by_segment_; }

    // Used in segment column index entry
    UniquePtr<IndexFileWorker> CreateFileWorker(CreateIndexParam *param, u32 segment_id);

private:
    Status CreateIndexDo(const ColumnDef *column_def, HashMap<u32, atomic_u64> &create_index_idxes);

    static SharedPtr<String> DetermineIndexDir(const String &parent_dir, const String &index_name);
    void CommitCreatedIndex(u32 segment_id, UniquePtr<SegmentColumnIndexEntry> index_entry);
    static String IndexFileName(const String &index_name, u32 segment_id);

private:
    std::shared_mutex rw_locker_{};

    TableIndexEntry *table_index_entry_{};
    ColumnID column_id_{};
    SharedPtr<String> col_index_dir_{}; // xxxx/col1
    const SharedPtr<IndexBase> index_base_{};
    HashMap<u32, SharedPtr<SegmentColumnIndexEntry>> index_by_segment_{};
};
} // namespace infinity
