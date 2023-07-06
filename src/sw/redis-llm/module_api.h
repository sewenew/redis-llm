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

#ifndef SEWENEW_REDIS_LLM_MODULE_API_H
#define SEWENEW_REDIS_LLM_MODULE_API_H

#ifdef __cplusplus

extern "C" {

#endif

#include "redismodule.h"

#ifdef __cplusplus

}

#endif

#include <cassert>
#include <memory>
#include "sw/redis-llm/errors.h"

namespace sw::redis::llm::api {

template <typename ...Args>
void warning(RedisModuleCtx *ctx, const char *fmt, Args &&...args) {
    RedisModule_Log(ctx, "warning", fmt, std::forward<Args>(args)...);
}

template <typename ...Args>
void notice(RedisModuleCtx *ctx, const char *fmt, Args &&...args) {
    RedisModule_Log(ctx, "notice", fmt, std::forward<Args>(args)...);
}

template <typename ...Args>
void debug(RedisModuleCtx *ctx, const char *fmt, Args &&...args) {
    RedisModule_Log(ctx, "debug", fmt, std::forward<Args>(args)...);
}

template <typename ...Args>
void verbose(RedisModuleCtx *ctx, const char *fmt, Args &&...args) {
    RedisModule_Log(ctx, "verbose", fmt, std::forward<Args>(args)...);
}

struct RedisKeyCloser {
    void operator()(RedisModuleKey *key) const {
        RedisModule_CloseKey(key);
    }
};

using RedisKey = std::unique_ptr<RedisModuleKey, RedisKeyCloser>;

enum class KeyMode {
    READONLY,
    WRITEONLY,
    READWRITE
};

RedisKey open_key(RedisModuleCtx *ctx, RedisModuleString *name, KeyMode mode);

enum class CreateOption {
    NX = 0,
    XX,
    NONE
};

RedisKey create_key(RedisModuleCtx *ctx, RedisModuleString *name,
        RedisModuleType *type, CreateOption opt);

// If key doesn't exist return false.
// If key type is NOT *key_type*, throw WrongTypeError.
// Otherwise, return true.
bool key_exists(RedisModuleKey *key, RedisModuleType *key_type);

int reply_with_error(RedisModuleCtx *ctx, const Error &err);

template <typename T>
T* get_value_by_key(RedisModuleKey &key) {
    auto *val = static_cast<T *>(RedisModule_ModuleTypeGetValue(&key));
    if (val == nullptr) {
        throw Error("failed to get value by key");
    }

    return val;
}

template <typename T>
T* get_value_by_key(RedisModuleCtx *ctx, RedisModuleString *key_name,
        RedisModuleType *type, KeyMode mode = KeyMode::READONLY) {
    assert(ctx != nullptr && key_name != nullptr && type != nullptr);

    auto key = open_key(ctx, key_name, mode);
    assert(key);

    if (!key_exists(key.get(), type)) {
        return nullptr;
    }

    auto *value = get_value_by_key<T>(*key);
    assert(value != nullptr);

    return value;
}

template <typename T>
T* get_value_by_key(RedisModuleCtx *ctx, const std::string &key_name,
        RedisModuleType *type, KeyMode mode = KeyMode::READONLY) {
    assert(ctx != nullptr && type != nullptr);

    auto *key_str = RedisModule_CreateString(ctx, key_name.data(), key_name.size());
    T *value = nullptr;
    try {
        value = get_value_by_key<T>(ctx, key_str, type, mode);
    } catch (...) {
        RedisModule_FreeString(ctx, key_str);
        throw;
    }

    RedisModule_FreeString(ctx, key_str);

    return value;
}

}

#endif // end SEWENEW_REDIS_LLM_MODULE_API_H
