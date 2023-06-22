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

#ifndef SEWENEW_REDIS_LLM_REDIS_LLM_H
#define SEWENEW_REDIS_LLM_REDIS_LLM_H

#include <unordered_set>
#include <nlohmann/json.hpp>
#include "sw/redis-llm/application.h"
#include "sw/redis-llm/embedding_model.h"
#include "sw/redis-llm/llm_model.h"
#include "sw/redis-llm/object.h"
#include "sw/redis-llm/options.h"
#include "sw/redis-llm/vector_store.h"
#include "sw/redis-llm/worker_pool.h"

namespace sw::redis::llm {

class RedisLlm {
public:
    static RedisLlm& instance();

    RedisLlm(const RedisLlm &) = delete;
    RedisLlm& operator=(const RedisLlm &) = delete;

    RedisLlm(RedisLlm &&) = delete;
    RedisLlm& operator=(RedisLlm &&) = delete;

    void load(RedisModuleCtx *ctx, RedisModuleString **argv, int argc);

    int module_version() const {
        return _MODULE_VERSION;
    }

    int encoding_version() const {
        return _ENCODING_VERSION;
    }

    const std::string& module_name() const {
        return _MODULE_NAME;
    }

    const std::string& llm_type_name() const {
        return _LLM_TYPE_NAME;
    }

    RedisModuleType* llm_type() {
        return _llm_module_type;
    }

    RedisModuleType* vector_store_type() {
        return _vector_store_module_type;
    }

    const std::string& vector_store_type_name() const {
        return _VECTOR_STORE_TYPE_NAME;
    }

    const std::string& app_type_name() const {
        return _APP_TYPE_NAME;
    }

    RedisModuleType* app_type() {
        return _app_module_type;
    }

    const Options& options() const {
        return _options;
    }

    LlmModelSPtr create_llm(const std::string &type, const nlohmann::json &conf);

    EmbeddingModelSPtr create_embedding(const std::string &type, const nlohmann::json &conf);

    VectorStoreSPtr create_vector_store(const std::string &type, const nlohmann::json &conf, LlmInfo &llm);

    ApplicationSPtr create_application(const std::string &type, const LlmInfo &llm, const nlohmann::json &conf);

    void unregister_object(const ObjectSPtr &obj) {
        _object_pool.erase(obj);
    }

    WorkerPool& worker_pool() {
        return *_worker_pool;
    }

private:
    RedisLlm() = default;

    void _register_object(const ObjectSPtr &obj) {
        _object_pool.insert(obj);
    }

    static void* _rdb_load_llm(RedisModuleIO *rdb, int encver);

    static void _rdb_save_llm(RedisModuleIO *rdb, void *value);

    static void _aof_rewrite_llm(RedisModuleIO *aof, RedisModuleString *key, void *value);

    static void _free_llm(void *value);

    static void* _rdb_load_app(RedisModuleIO *rdb, int encver);

    static void _rdb_save_app(RedisModuleIO *rdb, void *value);

    static void _aof_rewrite_app(RedisModuleIO *aof, RedisModuleString *key, void *value);

    static void _free_app(void *value);

    static void* _rdb_load_vector_store(RedisModuleIO *rdb, int encver);

    static void _rdb_save_vector_store(RedisModuleIO *rdb, void *value);

    static void _aof_rewrite_vector_store(RedisModuleIO *aof, RedisModuleString *key, void *value);

    static void _free_vector_store(void *value);

    const int _MODULE_VERSION = 1;

    const int _ENCODING_VERSION = 0;

    const std::string _MODULE_NAME = "LLM";

    const std::string _LLM_TYPE_NAME = "LLMMOD-SW";

    RedisModuleType *_llm_module_type = nullptr;

    const std::string _APP_TYPE_NAME = "LLMAPP-SW";

    RedisModuleType *_app_module_type = nullptr;

    const std::string _VECTOR_STORE_TYPE_NAME = "LLMVEC-SW";

    RedisModuleType *_vector_store_module_type = nullptr;

    Options _options;

    LlmModelFactory _llm_factory;

    EmbeddingModelFactory _embedding_factory;

    VectorStoreFactory _vector_store_factory;

    ApplicationFactory _app_factory;

    std::unique_ptr<WorkerPool> _worker_pool;

    std::unordered_set<ObjectSPtr> _object_pool;
};

}

#endif // end SEWENEW_REDIS_LLM_REDIS_LLM_H
