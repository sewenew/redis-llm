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

#include "sw/redis-llm/chat_application.h"
#include "sw/redis-llm/redis_llm.h"

namespace sw::redis::llm {

ChatApplication::ChatApplication(const LlmInfo &llm,
        const nlohmann::json &conf) :
    Application("chat", llm, conf),
    _system_prompt(conf.value<std::string>("prompt", _default_prompt)),
    _chat_history(_create_chat_history(conf.value<nlohmann::json>("history", {}))),
    _vector_store(conf.at("vector-store").get<std::string>()) {}

std::string ChatApplication::run(RedisModuleBlockedClient *blocked_client, LlmModel &model, const nlohmann::json &context, const std::string_view &input, bool verbose) {
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

    std::lock_guard<std::mutex> lock(_mtx);

    auto [summary, recent_history] = _chat_history.history(*llm_model, *vector_store, input);

    nlohmann::json vars;
    vars["history"] = summary;
    auto system_msg = _system_prompt.render(vars);

    auto reply = model.chat(input, system_msg, recent_history, {});

    auto user_res = _chat_history.add(*llm_model, *vector_store, "user", input);
    auto assistant_res = _chat_history.add(*llm_model, *vector_store, "assistant", reply);
    auto user_id = std::get<0>(user_res);
    auto assistant_id = std::get<0>(assistant_res);
    if (user_id > 0 || assistant_id > 0) {
        auto *ctx = RedisModule_GetThreadSafeContext(blocked_client);
        RedisModule_ThreadSafeContextLock(ctx);

        if (user_id > 0) {
            auto [id, data, embedding] = user_res;
            auto embedding_str = util::dump_embedding(embedding);
            auto id_str = std::to_string(id);
            RedisModule_Replicate(ctx, "LLM.ADD", "bcbcbb",
                    _vector_store.data(), _vector_store.size(),
                    "--ID", id_str.data(), id_str.size(),
                    "--EMBEDDING", embedding_str.data(), embedding_str.size(),
                    data.data(), data.size());
        }

        if (assistant_id > 0) {
            auto [id, data, embedding] = assistant_res;
            auto embedding_str = util::dump_embedding(embedding);
            auto id_str = std::to_string(id);
            RedisModule_Replicate(ctx, "LLM.ADD", "bcbcbb",
                    _vector_store.data(), _vector_store.size(),
                    "--ID", id_str.data(), id_str.size(),
                    "--EMBEDDING", embedding_str.data(), embedding_str.size(),
                    data.data(), data.size());
        }

        RedisModule_ThreadSafeContextUnlock(ctx);
        RedisModule_FreeThreadSafeContext(ctx);
    }

    return reply;
}

ChatHistory ChatApplication::_create_chat_history(const nlohmann::json &conf) const {
    ChatHistoryOptions opts(conf);
    opts.llm = llm();

    return ChatHistory(opts);
}

VectorStore& ChatApplication::_get_vector_store(RedisModuleCtx *ctx, const nlohmann::json &context) {
    auto *store = api::get_value_by_key<VectorStore>(ctx, _vector_store, RedisLlm::instance().vector_store_type());
    if (store == nullptr) {
        throw Error("vector store does not exist");
    }

    return *store;
}

}
