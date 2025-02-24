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


#include <sstream>
#include <iomanip>

import stl;

module utility;

namespace infinity::Utility {

SizeT NextPowerOfTwo(SizeT input) {
    --input;
    input |= input >> 1;
    input |= input >> 2;
    input |= input >> 4;
    input |= input >> 8;
    input |= input >> 16;
    input |= input >> 32;
    return ++input;
}

String FormatByteSize(u64 byte_size) {
    static const char* sizeSuffixes[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};

    if (byte_size == 0) {
        return "0B";
    }

    int suffixIndex = static_cast<int>(std::log2(byte_size) / 10);
    double size = static_cast<double>(byte_size) / (1 << (suffixIndex * 10));

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << sizeSuffixes[suffixIndex];
    return oss.str();
}


} // namespace infinity
