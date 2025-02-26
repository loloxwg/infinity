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

import stl;
import txn;
import query_context;
import table_def;
import data_table;
import parser;
import physical_operator_type;
import operator_state;
import status;
import infinity_exception;

module physical_drop_table;

namespace infinity {

void PhysicalDropTable::Init() {}

bool PhysicalDropTable::Execute(QueryContext *query_context, OperatorState *operator_state) {
    auto txn = query_context->GetTxn();

    Status status = txn->DropTableCollectionByName(*schema_name_, *table_name_, conflict_type_);

    if(!status.ok()) {
        operator_state->status_ = status;
    }

    // Generate the result
    Vector<SharedPtr<ColumnDef>> column_defs = {
        MakeShared<ColumnDef>(0, MakeShared<DataType>(LogicalType::kInteger), "OK", HashSet<ConstraintType>())};

    auto result_table_def_ptr = MakeShared<TableDef>(MakeShared<String>("default"), MakeShared<String>("Tables"), column_defs);
    output_ = MakeShared<DataTable>(result_table_def_ptr, TableType::kDataTable);
    operator_state->SetComplete();
    return true;
}

} // namespace infinity
