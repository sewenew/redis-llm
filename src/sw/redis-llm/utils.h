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
#include "nlohmann/json.hpp"
#include "sw/redis-llm/module_api.h"

namespace sw::redis::llm {

using Vector = std::vector<float>;

struct LlmInfo {
    LlmInfo() = default;

    explicit LlmInfo(const std::string_view &info);

    explicit LlmInfo(nlohmann::json info);

    std::string to_string() const;

    std::string key;

    nlohmann::json params;

private:
    void _init(nlohmann::json info);
};

namespace util {

std::string_view to_sv(RedisModuleString *str);

std::vector<std::string_view> to_sv(RedisModuleString **argv, int argc);

std::string to_string(RedisModuleString *str);

nlohmann::json to_json(RedisModuleString *str);

bool str_case_equal(const std::string_view &s1, const std::string_view &s2);

template <typename Func>
int run_command(Func &&func, RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    try {
        return func(ctx, argv, argc);
    } catch (const WrongArityError &err) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }

    return REDISMODULE_ERR;
}

template <typename Iter>
void split(const std::string_view &str, const std::string &delimiter, Iter result) {
    if (str.empty()) {
        return;
    }
    if (delimiter.empty()) {
        throw std::runtime_error("delimiter must be not empty");
    }

    std::string::size_type pos = 0;
    std::string::size_type idx = 0;
    while (true) {
        pos = str.find(delimiter, idx);
        if (pos == std::string::npos) {
            *result = str.substr(idx);
            ++result;
            break;
        }

        *result = str.substr(idx, pos - idx);
        ++result;
        idx = pos + delimiter.size();
    }
}

Vector parse_embedding(const std::string_view &opt);

std::string dump_embedding(const Vector &embedding);

}

}

#endif // end SEWENEW_REDIS_LLM_UTILS_H
