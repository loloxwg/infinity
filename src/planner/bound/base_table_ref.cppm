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

#include <vector>
import stl;
import table_ref;
import catalog;
import parser;
import table_function;
import block_index;

import infinity_exception;

export module base_table_ref;

namespace infinity {

export class BaseTableRef : public TableRef {
public:
    explicit BaseTableRef(TableEntry *table_entry_ptr,
                          Vector<SizeT> column_ids,
                          SharedPtr<BlockIndex> block_index,
                          const String &alias,
                          u64 table_index,
                          SharedPtr<Vector<String>> column_names,
                          SharedPtr<Vector<SharedPtr<DataType>>> column_types)
        : TableRef(TableRefType::kTable, alias), table_entry_ptr_(table_entry_ptr), column_ids_(std::move(column_ids)),
          block_index_(std::move(block_index)), column_names_(std::move(column_names)), column_types_(std::move(column_types)), table_index_(table_index) {}

    static BaseTableRef FakeTableRef(TableEntry *table_entry, TxnTimeStamp ts) {
        SharedPtr<BlockIndex> block_index = table_entry->GetBlockIndex(ts);
        return BaseTableRef(table_entry, {}, block_index, "", 0, {}, {});
    }

    void RetainColumnByIndices(const Vector<SizeT> &&indices) {
        replace_field<SizeT>(column_ids_, indices);
        replace_field<String>(*column_names_, indices);
        replace_field<SharedPtr<DataType>>(*column_types_, indices);
    };

    SharedPtr<String> schema_name() const { return table_entry_ptr_->GetDBName(); }

    SharedPtr<String> table_name() const { return table_entry_ptr_->GetTableName(); }

    TableEntry *table_entry_ptr_{};
    Vector<SizeT> column_ids_{};
    SharedPtr<BlockIndex> block_index_{};

    SharedPtr<Vector<String>> column_names_{};
    SharedPtr<Vector<SharedPtr<DataType>>> column_types_{};
    u64 table_index_;

private:
    template <typename T>
    void replace_field(Vector<T> &field, const Vector<SizeT> &indices) {
        Vector<T> items;
        items.reserve(field.size());

        for (const auto &i : indices) {
            items.push_back(field[i]);
        }
        field = items;
    }
};

} // namespace infinity