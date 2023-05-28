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

#include "sw/redis-llm/redis_llm.h"
#include <cassert>
#include <string>
#include "sw/redis-llm/application.h"
#include "sw/redis-llm/command.h"
#include "sw/redis-llm/errors.h"
#include "nlohmann/json.hpp"

namespace {

using namespace sw::redis::llm;

struct StringDeleter {
    void operator()(char *str) const {
        if (str != nullptr) {
            RedisModule_Free(str);
        }
    }
};

using StringUPtr = std::unique_ptr<char, StringDeleter>;

struct RDBString {
    StringUPtr str;
    std::size_t len;
};

RDBString rdb_load_string(RedisModuleIO *rdb);

uint64_t rdb_load_number(RedisModuleIO *rdb);

Application* rdb_load_application(RedisModuleIO *rdb);

void rdb_save_string(RedisModuleIO *rdb, const std::string_view &str);

void rdb_save_number(RedisModuleIO *rdb, uint64_t num);

void rdb_save_vector(RedisModuleIO *rdb, const Vector &vec);

void rdb_save_application(RedisModuleIO *rdb, Application &app);

void rewrite_conf(RedisModuleIO *aof, RedisModuleString *key, const nlohmann::json &conf);

void rewrite_data(RedisModuleIO *aof, RedisModuleString *key, Application &app);

}

namespace sw::redis::llm {

RedisLlm& RedisLlm::instance() {
    static RedisLlm redis_llm;

    return redis_llm;
}

void RedisLlm::load(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    assert(ctx != nullptr);

    if (RedisModule_Init(ctx,
                module_name().data(),
                module_version(),
                REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        throw Error("fail to init module of " + module_name() + " type");
    }

    _options.load(argv, argc);

    RedisModuleTypeMethods methods = {
        REDISMODULE_TYPE_METHOD_VERSION,
        _rdb_load,
        _rdb_save,
        _aof_rewrite,
        nullptr,
        nullptr,
        _free_msg
    };

    _module_type = RedisModule_CreateDataType(ctx,
            type_name().data(),
            encoding_version(),
            &methods);
    if (_module_type == nullptr) {
        throw Error(std::string("failed to create ") + type_name() + " type");
    }

    cmd::create_commands(ctx);
}

void* RedisLlm::_rdb_load(RedisModuleIO *rdb, int encver) {
    try {
        assert(rdb != nullptr);

        auto &m = RedisLlm::instance();

        if (encver != m.encoding_version()) {
            throw Error("cannot load data of version: " + std::to_string(encver));
        }

        return rdb_load_application(rdb);
    } catch (const Error &e) {
        RedisModule_LogIOError(rdb, "warning", e.what());
        return nullptr;
    }
}

void RedisLlm::_rdb_save(RedisModuleIO *rdb, void *value) {
    try {
        assert(rdb != nullptr);

        if (value == nullptr) {
            throw Error("null value to save");
        }

        auto *app = static_cast<Application *>(value);

        rdb_save_application(rdb, *app);
    } catch (const Error &e) {
        RedisModule_LogIOError(rdb, "warning", e.what());
    }
}

void RedisLlm::_aof_rewrite(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    try {
        assert(aof != nullptr);

        if (key == nullptr || value == nullptr) {
            throw Error("null key or value to rewrite aof");
        }

        auto *app = static_cast<Application *>(value);
        rewrite_conf(aof, key, app->conf());

        rewrite_data(aof, key, *app);

    } catch (const Error &e) {
        RedisModule_LogIOError(aof, "warning", e.what());
    }
}

void RedisLlm::_free_msg(void *value) {
    if (value != nullptr) {
        auto *app = static_cast<Application *>(value);
        delete app;
    }
}

}

namespace {

using namespace sw::redis::llm;

std::string_view to_sv(RDBString rdb_str) {
    return {rdb_str.str.get(), rdb_str.len};
}

RDBString rdb_load_string(RedisModuleIO *rdb) {
    std::size_t len = 0;
    auto *buf = RedisModule_LoadStringBuffer(rdb, &len);
    if (buf == nullptr) {
        throw Error("failed to load string buffer from rdb");
    }

    return {StringUPtr(buf), len};
}

uint64_t rdb_load_number(RedisModuleIO *rdb) {
    return RedisModule_LoadUnsigned(rdb);
}

Vector rdb_load_vector(RedisModuleIO *rdb, uint64_t dim) {
    Vector vec;
    vec.reserve(dim);
    for (auto idx = 0UL; idx < dim; ++idx) {
        vec.push_back(RedisModule_LoadFloat(rdb));
    }

    return vec;
}

nlohmann::json rdb_load_config(RedisModuleIO *rdb) {
    auto config = rdb_load_string(rdb);
    nlohmann::json conf;
    try {
        auto beg = config.str.get();
        auto end = beg + config.len;
        conf = nlohmann::json::parse(beg, end);
    } catch (const nlohmann::json::exception &e) {
        throw Error("failed parse config");
    }

    return conf;
}

void rdb_save_config(RedisModuleIO *rdb, const nlohmann::json &conf) {
    std::string config;
    try {
        config = conf.dump();
    } catch (const nlohmann::json::exception &e) {
        throw Error("failed to dump config");
    }

    RedisModule_SaveStringBuffer(rdb, config.data(), config.size());
}

void rdb_save_number(RedisModuleIO *rdb, uint64_t num) {
    RedisModule_SaveUnsigned(rdb, num);
}

void rdb_save_string(RedisModuleIO *rdb, const std::string_view &str) {
    RedisModule_SaveStringBuffer(rdb, str.data(), str.size());
}

void rdb_save_vector(RedisModuleIO *rdb, const Vector &vec) {
    for (auto ele : vec) {
        RedisModule_SaveFloat(rdb, ele);
    }
}

void rdb_save_data_store(RedisModuleIO *rdb, Application &app) {
    const auto &data_store = app.data_store();
    auto &vector_store = app.vector_store();
    rdb_save_number(rdb, data_store.size());
    rdb_save_number(rdb, app.dim());

    for (auto &[id, data] : data_store) {
        rdb_save_number(rdb, id);
        rdb_save_string(rdb, data);

        auto vec = vector_store.get(id);
        // TODO: is it possible that vec is nullopt?
        rdb_save_vector(rdb, *vec);
    }
}

void rdb_save_application(RedisModuleIO *rdb, Application &app) {
    rdb_save_config(rdb, app.conf());

    rdb_save_data_store(rdb, app);
}

auto parse_config(const nlohmann::json &config) ->
    std::tuple<nlohmann::json, nlohmann::json, nlohmann::json > {
    auto iter = config.find("llm");
    if (iter == config.end()) {
        throw Error("no llm config");
    }
    auto llm_config = iter.value();

    nlohmann::json embedding_config;
    iter = config.find("embedding");
    if (iter != config.end()) {
        embedding_config = iter.value();
    }

    nlohmann::json vector_store_config;
    iter = config.find("vector_store");
    if (iter != config.end()) {
        vector_store_config = iter.value();
    }

    return {std::move(llm_config), std::move(embedding_config), std::move(vector_store_config)};
}

auto rdb_load_data_store(RedisModuleIO *rdb) ->
    std::unordered_map<uint64_t, std::pair<std::string, Vector>> {
    std::unordered_map<uint64_t, std::pair<std::string, Vector>> data_store;
    auto size = rdb_load_number(rdb);
    auto dim = rdb_load_number(rdb);
    for (auto idx = 0UL; idx < size; ++idx) {
        auto id = rdb_load_number(rdb);
        auto val = to_sv(rdb_load_string(rdb));
        auto vec = rdb_load_vector(rdb, dim);
        data_store.emplace(id, std::make_pair(val, std::move(vec)));
    }

    return data_store;
}

Application* rdb_load_application(RedisModuleIO *rdb) {
    auto config = rdb_load_config(rdb);
    auto [llm_config, embedding_config, vector_store_config] = parse_config(config);

    auto data_store = rdb_load_data_store(rdb);

    return new Application(llm_config,
            embedding_config,
            vector_store_config,
            std::move(data_store));
}

void rewrite_conf(RedisModuleIO *aof, RedisModuleString *key, const nlohmann::json &conf) {
    auto llm_config = conf.at("llm").dump();
    auto embedding_config = conf.at("embedding").dump();
    auto vector_store_config = conf.at("vector_store").dump();

    RedisModule_EmitAOF(aof,
            "LLM.CREATE",
            "scbcbcb",
            key,
            "--LLM",
            llm_config.data(),
            llm_config.size(),
            "--EMBEDDING",
            embedding_config.data(),
            embedding_config.size(),
            "--VECTOR_STORE",
            vector_store_config.data(),
            vector_store_config.size());
}

void rewrite_data(RedisModuleIO *aof, RedisModuleString *key, Application &app) {
    const auto &data_store = app.data_store();
    for (auto &[id, data] : data_store) {
        auto vec = app.embedding(id);
        if (!vec) {
            // TODO: this should not happen
            continue;
        }
        std::string embedding;
        for (auto ele : *vec) {
            if (!embedding.empty()) {
                embedding += ",";
            }
            embedding += std::to_string(ele);
        }
        auto id_str = std::to_string(id);

        RedisModule_EmitAOF(aof,
                "LLM.ADD",
                "scbbb",
                key,
                "--WITHEMBEDDING",
                id_str.data(),
                id_str.size(),
                data.data(),
                data.size(),
                embedding.data(),
                embedding.size());
    }
}

}
