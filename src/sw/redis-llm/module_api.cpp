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

#include "sw/redis-llm/module_api.h"
#include <cassert>

namespace sw::redis::llm::api {

RedisKey open_key(RedisModuleCtx *ctx, RedisModuleString *name, KeyMode key_mode) {
    if (name == nullptr) {
        throw Error("cannot open key with a null name");
    }

    int mode = 0;
    switch (key_mode) {
    case KeyMode::READONLY:
        mode = REDISMODULE_READ;
        break;

    case KeyMode::WRITEONLY:
        mode = REDISMODULE_WRITE;
        break;

    case KeyMode::READWRITE:
        mode = REDISMODULE_READ | REDISMODULE_WRITE;
        break;

    default:
        assert(false);
    }

    return RedisKey(static_cast<RedisModuleKey *>(RedisModule_OpenKey(ctx, name, mode)));
}

bool key_exists(RedisModuleKey *key, RedisModuleType *key_type) {
    // key can be nullptr.
    auto type = RedisModule_KeyType(key);
    if (type == REDISMODULE_KEYTYPE_EMPTY) {
        return false;
    }

    if (RedisModule_ModuleTypeGetType(key) == key_type) {
        return true;
    }

    throw WrongTypeError(REDISMODULE_ERRORMSG_WRONGTYPE);
}

int reply_with_error(RedisModuleCtx *ctx, const Error &err) {
    auto msg = std::string("ERR ") + err.what();

    return RedisModule_ReplyWithError(ctx, msg.data());
}

LlmModel* get_model_by_key(RedisModuleCtx *ctx, const std::string &key_name, RedisModuleType *type) {
    auto *key_str = RedisModule_CreateString(ctx, key_name.data(), key_name.size());
    LlmModel *model = nullptr;
    try {
        model = get_model_by_key(ctx, key_str, type);
    } catch (...) {
        RedisModule_FreeString(ctx, key_str);
        throw;
    }

    RedisModule_FreeString(ctx, key_str);

    return model;
}

LlmModel* get_model_by_key(RedisModuleCtx *ctx, RedisModuleString *key_name, RedisModuleType *type) {
    assert(ctx != nullptr && type != nullptr);

    auto key = open_key(ctx, key_name, api::KeyMode::READONLY);
    assert(key);

    if (!key_exists(key.get(), type)) {
        return nullptr;
    }

    auto *model = api::get_value_by_key<LlmModel>(*key);
    assert(model != nullptr);

    return model;
}

}
