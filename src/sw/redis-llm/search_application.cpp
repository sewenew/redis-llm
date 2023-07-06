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

#include "sw/redis-llm/search_application.h"
#include "sw/redis-llm/redis_llm.h"

namespace sw::redis::llm {

SearchApplication::SearchApplication(const LlmInfo &llm,
        const nlohmann::json &conf) :
    Application("search", llm, conf),
    _prompt(conf.value<std::string>("prompt", _default_prompt)),
    _vector_store(conf.at("vector-store").get<std::string>()),
    _k(conf.at("k").get<std::size_t>()) {}

std::string SearchApplication::run(RedisModuleBlockedClient *blocked_client, LlmModel &model, const nlohmann::json &context, const std::string_view &input, bool verbose) {
    /*
    if (context.is_null()) {
        throw Error("no context is specified");
    }
    */

    VectorStoreSPtr vector_store;
    LlmModelSPtr llm_model;

    auto *ctx = RedisModule_GetThreadSafeContext(blocked_client);
    RedisModule_ThreadSafeContextLock(ctx);

    try {
        auto &store = _get_vector_store(ctx, context);
        vector_store = std::static_pointer_cast<VectorStore>(store.shared_from_this());
        auto *store_model = api::get_value_by_key<LlmModel>(ctx, store.llm().key, RedisLlm::instance().llm_type());
        if (store_model == nullptr) {
            throw Error("LLM model does not exist: " + store.llm().key);
        }
        llm_model = std::static_pointer_cast<LlmModel>(store_model->shared_from_this());
    } catch (const Error &) {
        RedisModule_ThreadSafeContextUnlock(ctx);
        RedisModule_FreeThreadSafeContext(ctx);
        throw;
    }

    RedisModule_ThreadSafeContextUnlock(ctx);
    RedisModule_FreeThreadSafeContext(ctx);

    auto embedding = llm_model->embedding(input, vector_store->llm().params);

    auto similar_items = _get_similar_items(embedding, *vector_store);

    nlohmann::json vars;
    if (!context.is_null()) {
        vars = context.value<nlohmann::json>("vars", nlohmann::json::object());
    }
    vars["question"] = std::string(input);
    std::string ctx_var;
    for (auto &item : similar_items) {
        if (!ctx_var.empty()) {
            ctx_var += "\n";
        }
        ctx_var += item;
    }
    vars["context"] = ctx_var;
    auto request = _prompt.render(vars);

    std::string output;
    if (verbose) {
        output += request;
        output += "\n\n";
    }

    output += model.predict(request, llm().params);

    return output;
}

VectorStore& SearchApplication::_get_vector_store(RedisModuleCtx *ctx, const nlohmann::json &context) {
    auto *store = api::get_value_by_key<VectorStore>(ctx, _vector_store, RedisLlm::instance().vector_store_type());
    if (store == nullptr) {
        throw Error("vector store does not exist");
    }

    return *store;
}

std::vector<std::string> SearchApplication::_get_similar_items(const Vector &embedding, VectorStore &store) {
    auto neighbors = store.knn(embedding, _k);
    std::vector<std::string> items;
    for (auto &ele : neighbors) {
        auto item = store.data(ele.first);
        if (item) {
            items.push_back(std::move(*item));
        }
    }

    return items;
}

}
