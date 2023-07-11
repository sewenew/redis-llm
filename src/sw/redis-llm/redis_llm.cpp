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
#include <string_view>
#include "sw/redis-llm/command.h"
#include "sw/redis-llm/errors.h"
#include "sw/redis-llm/module_api.h"
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

void rdb_save_string(RedisModuleIO *rdb, const std::string_view &str);

void rdb_save_number(RedisModuleIO *rdb, uint64_t num);

void rdb_save_vector(RedisModuleIO *rdb, const Vector &vec);

void* rdb_load_llm(RedisModuleIO *rdb);

void rdb_save_llm(RedisModuleIO *rdb, void *value);

void* rdb_load_app(RedisModuleIO *rdb);

void rdb_save_app(RedisModuleIO *rdb, void *value);

void rewrite_vector_store(RedisModuleIO *aof, RedisModuleString *key, VectorStore &store);

void rdb_save_vector_store(RedisModuleIO *rdb, VectorStore &store);

void rdb_load_vector_store(RedisModuleIO *rdb, VectorStore &store, std::size_t dim);

void* rdb_load_vector_store(RedisModuleIO *rdb);

void rdb_save_config(RedisModuleIO *rdb, const nlohmann::json &conf);

nlohmann::json rdb_load_config(RedisModuleIO *rdb);

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

    _worker_pool = std::make_unique<WorkerPool>(_options.worker_pool_opts);

    RedisModuleTypeMethods llm_methods = {
        REDISMODULE_TYPE_METHOD_VERSION,
        _rdb_load_llm,
        _rdb_save_llm,
        _aof_rewrite_llm,
        nullptr,
        nullptr,
        _free_llm
    };

    _llm_module_type = RedisModule_CreateDataType(ctx,
            llm_type_name().data(),
            encoding_version(),
            &llm_methods);
    if (_llm_module_type == nullptr) {
        throw Error(std::string("failed to create ") + llm_type_name() + " type");
    }

    RedisModuleTypeMethods app_methods = {
        REDISMODULE_TYPE_METHOD_VERSION,
        _rdb_load_app,
        _rdb_save_app,
        _aof_rewrite_app,
        nullptr,
        nullptr,
        _free_app
    };

    _app_module_type = RedisModule_CreateDataType(ctx,
            app_type_name().data(),
            encoding_version(),
            &app_methods);
    if (_app_module_type == nullptr) {
        throw Error(std::string("failed to create ") + app_type_name() + " type");
    }

    RedisModuleTypeMethods vector_store_methods = {
        REDISMODULE_TYPE_METHOD_VERSION,
        _rdb_load_vector_store,
        _rdb_save_vector_store,
        _aof_rewrite_vector_store,
        nullptr,
        nullptr,
        _free_vector_store
    };

    _vector_store_module_type = RedisModule_CreateDataType(ctx,
            vector_store_type_name().data(),
            encoding_version(),
            &vector_store_methods);
    if (_vector_store_module_type == nullptr) {
        throw Error(std::string("failed to create ") + vector_store_type_name() + " type");
    }

    cmd::create_commands(ctx);
}

LlmModelSPtr RedisLlm::create_llm(const std::string &type, const nlohmann::json &conf) {
    auto model = _llm_factory.create(type, conf);

    _register_object(model);

    return model;
}

EmbeddingModelSPtr RedisLlm::create_embedding(const std::string &type, const nlohmann::json &conf) {
    auto embedding = _embedding_factory.create(type, conf);

    _register_object(embedding);

    return embedding;
}

VectorStoreSPtr RedisLlm::create_vector_store(const std::string &type, const nlohmann::json &conf, LlmInfo &llm) {
    auto store = _vector_store_factory.create(type, conf, llm);

    _register_object(store);

    return store;
}

ApplicationSPtr RedisLlm::create_application(const std::string &type, const LlmInfo &llm, const nlohmann::json &conf) {
    auto app = _app_factory.create(type, llm, conf);

    _register_object(app);

    return app;
}

void* RedisLlm::_rdb_load_llm(RedisModuleIO *rdb, int encver) {
    try {
        assert(rdb != nullptr);

        auto &m = RedisLlm::instance();

        if (encver != m.encoding_version()) {
            throw Error("cannot load data of version: " + std::to_string(encver));
        }

        return rdb_load_llm(rdb);
    } catch (const Error &e) {
        RedisModule_LogIOError(rdb, "warning", e.what());
        return nullptr;
    }
}

void RedisLlm::_rdb_save_llm(RedisModuleIO *rdb, void *value) {
    try {
        assert(rdb != nullptr);

        if (value == nullptr) {
            throw Error("null value to save");
        }

        rdb_save_llm(rdb, value);
    } catch (const Error &e) {
        RedisModule_LogIOError(rdb, "warning", e.what());
    }
}

void RedisLlm::_aof_rewrite_llm(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    try {
        assert(aof != nullptr);

        if (key == nullptr || value == nullptr) {
            throw Error("null key or value to rewrite aof");
        }

        auto *model = static_cast<LlmModel *>(value);
        const auto &type = model->type();
        std::string conf;
        try {
            conf = model->conf().dump();
        } catch (const std::exception &e) {
            throw Error(std::string("failed to dump LLM model conf: ") + e.what());
        }

        RedisModule_EmitAOF(aof,
                "LLM.CREATE-LLM",
                "scbcb",
                key,
                "--TYPE",
                type.data(),
                type.size(),
                "--PARAMS",
                conf.data(),
                conf.size());
    } catch (const Error &e) {
        RedisModule_LogIOError(aof, "warning", e.what());
    }
}

void RedisLlm::_free_llm(void *value) {
    if (value != nullptr) {
        auto *llm = static_cast<LlmModel *>(value);
        auto obj = llm->shared_from_this();
        instance().unregister_object(obj);
    }
}

void* RedisLlm::_rdb_load_vector_store(RedisModuleIO *rdb, int encver) {
    try {
        assert(rdb != nullptr);

        auto &m = RedisLlm::instance();

        if (encver != m.encoding_version()) {
            throw Error("cannot load data of version: " + std::to_string(encver));
        }

        return rdb_load_vector_store(rdb);
    } catch (const Error &e) {
        RedisModule_LogIOError(rdb, "warning", e.what());
    }
    return nullptr;
}

void RedisLlm::_rdb_save_vector_store(RedisModuleIO *rdb, void *value) {
    try {
        assert(rdb != nullptr);

        if (value == nullptr) {
            throw Error("null value to save");
        }

        auto *store = static_cast<VectorStore *>(value);
        rdb_save_vector_store(rdb, *store);
    } catch (const Error &e) {
        RedisModule_LogIOError(rdb, "warning", e.what());
    }
}

void RedisLlm::_aof_rewrite_vector_store(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    try {
        assert(aof != nullptr);

        if (key == nullptr || value == nullptr) {
            throw Error("null key or value to rewrite aof");
        }

        auto *store = static_cast<VectorStore *>(value);
        const auto &type = store->type();
        std::string conf;
        try {
            conf = store->conf().dump();
        } catch (const std::exception &e) {
            throw Error(std::string("failed to dump LLM model conf: ") + e.what());
        }

        RedisModule_EmitAOF(aof,
                "LLM.CREATE-VECTOR-STORE",
                "scbcb",
                key,
                "--TYPE",
                type.data(),
                type.size(),
                "--PARAMS",
                conf.data(),
                conf.size());

        rewrite_vector_store(aof, key, *store);
    } catch (const Error &e) {
        RedisModule_LogIOError(aof, "warning", e.what());
    }
}

void RedisLlm::_free_vector_store(void *value) {
    if (value != nullptr) {
        auto *store = static_cast<VectorStore *>(value);
        auto obj = store->shared_from_this();
        instance().unregister_object(obj);
    }
}

void* RedisLlm::_rdb_load_app(RedisModuleIO *rdb, int encver) {
    try {
        assert(rdb != nullptr);

        auto &m = RedisLlm::instance();

        if (encver != m.encoding_version()) {
            throw Error("cannot load data of version: " + std::to_string(encver));
        }

        return rdb_load_app(rdb);
    } catch (const Error &e) {
        RedisModule_LogIOError(rdb, "warning", e.what());
    }
    return nullptr;
}

void RedisLlm::_rdb_save_app(RedisModuleIO *rdb, void *value) {
    try {
        assert(rdb != nullptr);

        if (value == nullptr) {
            throw Error("null value to save");
        }

        rdb_save_app(rdb, value);
    } catch (const Error &e) {
        RedisModule_LogIOError(rdb, "warning", e.what());
    }
}

void RedisLlm::_aof_rewrite_app(RedisModuleIO *aof, RedisModuleString *key, void *value) {
    try {
        assert(aof != nullptr);

        if (key == nullptr || value == nullptr) {
            throw Error("null key or value to rewrite aof");
        }

        auto *app = static_cast<Application *>(value);
        const auto &type = app->type();
        std::string llm;
        std::string conf;
        try {
            llm = app->llm().to_string();
            conf = app->conf().dump();
        } catch (const std::exception &e) {
            throw Error(std::string("failed to dump app conf: ") + e.what());
        }

        RedisModule_EmitAOF(aof,
                "LLM.CREATE-APP",
                "scbcbcb",
                key,
                "--TYPE",
                type.data(),
                type.size(),
                "--LLM",
                llm.data(),
                llm.size(),
                "--PARAMS",
                conf.data(),
                conf.size());
    } catch (const Error &e) {
        RedisModule_LogIOError(aof, "warning", e.what());
    }
}

void RedisLlm::_free_app(void *value) {
    if (value != nullptr) {
        auto *app = static_cast<Application *>(value);
        auto obj = app->shared_from_this();
        instance().unregister_object(obj);
    }
}

}

namespace {

using namespace sw::redis::llm;

std::string_view to_sv(RDBString &rdb_str) {
    return {rdb_str.str.get(), rdb_str.len};
}

std::string to_string(RDBString rdb_str) {
    return std::string(to_sv(rdb_str));
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

void rdb_save_llm(RedisModuleIO *rdb, void *value) {
    auto *model = static_cast<LlmModel *>(value);
    const auto &type = model->type();
    std::string conf;
    try {
        conf = model->conf().dump();
    } catch (const std::exception &e) {
        throw Error(std::string("failed to dump LLM model conf: ") + e.what());
    }

    rdb_save_string(rdb, type);
    rdb_save_string(rdb, conf);
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

void rdb_save_vector_store(RedisModuleIO *rdb, VectorStore &store) {
    rdb_save_string(rdb, store.type());
    rdb_save_config(rdb, store.conf());
    rdb_save_string(rdb, store.llm().to_string());
    rdb_save_number(rdb, store.id_idx());
    rdb_save_number(rdb, store.dim());

    const auto &data_store = store.data_store();
    rdb_save_number(rdb, data_store.size());

    for (auto &[id, data] : data_store) {
        rdb_save_number(rdb, id);
        rdb_save_string(rdb, data);

        auto vec = store.get(id);
        // TODO: is it possible that vec is nullopt?
        rdb_save_vector(rdb, *vec);
    }
}

void rdb_save_app(RedisModuleIO *rdb, void *value) {
    auto *app = static_cast<Application *>(value);

    rdb_save_string(rdb, app->type());
    rdb_save_string(rdb, app->llm().to_string());
    rdb_save_config(rdb, app->conf());
}

void rdb_load_vector_store(RedisModuleIO *rdb, VectorStore &store, std::size_t dim) {
    auto size = rdb_load_number(rdb);
    for (auto idx = 0UL; idx < size; ++idx) {
        auto id = rdb_load_number(rdb);
        auto val = to_string(rdb_load_string(rdb));
        auto vec = rdb_load_vector(rdb, dim);
        store.add(id, val, std::move(vec));
    }
}

void* rdb_load_llm(RedisModuleIO *rdb) {
    auto &llm = RedisLlm::instance();
    auto type = to_string(rdb_load_string(rdb));;
    auto conf = rdb_load_config(rdb);

    auto model = llm.create_llm(type, conf);

    return model.get();
}

void* rdb_load_app(RedisModuleIO *rdb) {
    auto type = to_string(rdb_load_string(rdb));
    auto info_str = rdb_load_string(rdb);
    LlmInfo llm_info(to_sv(info_str));
    auto conf = rdb_load_config(rdb);

    auto app = RedisLlm::instance().create_application(type, llm_info, conf);

    return app.get();
}

void* rdb_load_vector_store(RedisModuleIO *rdb) {
    auto &llm = RedisLlm::instance();
    std::string type = to_string(rdb_load_string(rdb));
    auto conf = rdb_load_config(rdb);
    auto info_str = rdb_load_string(rdb);
    LlmInfo llm_info(to_sv(info_str));
    auto id_idx = rdb_load_number(rdb);
    auto dim = rdb_load_number(rdb);

    auto store = llm.create_vector_store(type, conf, llm_info);
    store->set_id_idx(id_idx);

    rdb_load_vector_store(rdb, *store, dim);

    return store.get();
}

void rewrite_vector_store(RedisModuleIO *aof, RedisModuleString *key, VectorStore &store) {
    const auto &data_store = store.data_store();
    for (auto &[id, data] : data_store) {
        auto vec = store.get(id);
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
