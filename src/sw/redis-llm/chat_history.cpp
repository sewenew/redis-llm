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

#include "sw/redis-llm/chat_history.h"
#include "sw/redis-llm/errors.h"
#include "sw/redis-llm/redis_llm.h"

namespace sw::redis::llm {

ChatHistoryOptions::ChatHistoryOptions(const nlohmann::json &conf) {
    if (conf.is_null()) {
        return;
    }

    auto iter = conf.find("summary_cnt");
    if (iter != conf.end()) {
        summary_cnt = iter.value().get<uint32_t>();
    }

    iter = conf.find("summary_ctx_cnt");
    if (iter != conf.end()) {
        summary_ctx_cnt = iter.value().get<uint32_t>();
    }

    iter = conf.find("summary_prompt");
    if (iter != conf.end()) {
        summary_prompt = iter.value().get<std::string>();
    }

    iter = conf.find("msg_ctx_cnt");
    if (iter != conf.end()) {
        msg_ctx_cnt = iter.value().get<uint32_t>();
    }

    iter = conf.find("store_type");
    if (iter != conf.end()) {
        store_type = iter.value().get<std::string>();
    }

    iter = conf.find("store_params");
    if (iter != conf.end()) {
        store_params = iter.value().get<std::string>();
    }
}

ChatHistory::ChatHistory(const ChatHistoryOptions &opts) :
    _opts(opts), _summary_prompt(_opts.summary_prompt) {}

std::tuple<uint64_t, std::string, Vector> ChatHistory::add(LlmModel &model, VectorStore &store, const std::string_view &role, const std::string_view &message) {
    auto msg = Msg(role, message);

    auto res = _summarize_if_needed(model, store, msg);

    _latest_msgs.push_back(std::move(msg));
    while (_latest_msgs.size() > _opts.msg_ctx_cnt) {
        _latest_msgs.pop_front();
    }

    return res;
}

std::pair<std::string, nlohmann::json> ChatHistory::history(LlmModel &model, VectorStore &store, const std::string_view &input) {
    auto latest_msgs = _get_latest_msgs();

    /*
    nlohmann::json current_msg;
    current_msg["role"] = "user";
    current_msg["content"] = input;
    latest_msgs.push_back(std::move(current_msg));
    */

    std::string summary;
    if (_opts.summary_cnt > 0 && _opts.summary_ctx_cnt > 0) {
        // TODO: Use the latest N message as input to look up the vector store,
        // so that it can be more semantic.
        summary = _get_history_summary(model, store, input, _opts.summary_ctx_cnt);
    }

    return std::make_pair(std::move(summary), std::move(latest_msgs));
}

std::string ChatHistory::_get_history_summary(LlmModel &model, VectorStore &store, const std::string_view &input, int k) {
    auto embedding = model.embedding(input, _opts.llm.params);
    auto neighbors = store.knn(embedding, k);
    std::string res;
    for (auto [id, dist] : neighbors) {
        auto val = store.data(id);
        if (!val) {
            continue;
        }

        if (!res.empty()) {
            res += "\n";
        }
        res += *val;
    }

    return res;
}

nlohmann::json ChatHistory::_get_latest_msgs() {
    nlohmann::json msgs;
    for (const auto &[role, content] : _latest_msgs) {
        nlohmann::json msg;
        msg["role"] = role;
        msg["content"] = content;
        msgs.push_back(std::move(msg));
    }

    return msgs;
}

std::tuple<uint64_t, std::string, Vector> ChatHistory::_summarize_if_needed(LlmModel &model, VectorStore &store, const Msg &msg) {
    if (_opts.summary_cnt == 0) {
        // Not enabled.
        return std::make_tuple(0, std::string(""), Vector{});
    }

    _msgs_to_be_summarize.push_back(msg);

    // TODO: also check the token limit.
    std::tuple<uint64_t, std::string, Vector> res;
    if (_msgs_to_be_summarize.size() >= _opts.summary_cnt) {
        res = _summarize(model, store, _msgs_to_be_summarize);

        _msgs_to_be_summarize.clear();
    }

    return res;
}

std::tuple<uint64_t, std::string, Vector> ChatHistory::_summarize(LlmModel &model, VectorStore &store, const std::vector<Msg> &msgs) {
    std::string request;
    try {
        nlohmann::json conversation;
        for (const auto &[role, content] : msgs) {
            nlohmann::json msg;
            msg["role"] = role;
            msg["content"] = content;

            conversation.push_back(std::move(msg));
        }

        nlohmann::json vars;
        vars["conversation"] = conversation.dump();

        request = _summary_prompt.render(vars);
    } catch (const std::exception &e) {
        throw Error(std::string("failed to build summary request") + e.what());
    }

    auto output = model.predict(request, _opts.llm.params);

    auto embedding = model.embedding(output, _opts.llm.params);

    auto id = store.add(output, embedding);

    return std::make_tuple(id, output, embedding);
}

}
