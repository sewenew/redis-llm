/**************************************************************************
   Copyright (c) 2023 sewenew

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 *************************************************************************/

#ifndef SEWENEW_REDIS_LLM_UTILS_H
#define SEWENEW_REDIS_LLM_UTILS_H

#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include "sw/redis-llm/module_api.h"

namespace sw::redis::llm {

namespace util {

std::string_view to_sv(RedisModuleString *str);

bool str_case_equal(const std::string_view &s1, const std::string_view &s2);

int32_t sv_to_int32(const StringView &sv);

int64_t sv_to_int64(const StringView &sv);

uint32_t sv_to_uint32(const StringView &sv);

uint64_t sv_to_uint64(const StringView &sv);

double sv_to_double(const StringView &sv);

float sv_to_float(const StringView &sv);

bool sv_to_bool(const StringView &sv);

std::string sv_to_string(const StringView &sv);

}

namespace io {

bool is_regular(const std::string &file);

bool is_directory(const std::string &file);

std::vector<std::string> list_dir(const std::string &path);

std::string extension(const std::string &file);

void remove_file(const std::string &path);

}

}

}

}

#endif // end SEWENEW_REDIS_LLM_UTILS_H
