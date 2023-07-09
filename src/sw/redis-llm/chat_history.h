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

#ifndef SEWENEW_REDIS_LLM_CHAT_HISTORY_H
#define SEWENEW_REDIS_LLM_CHAT_HISTORY_H

#include <tuple>
#include <string>
#include <string_view>
#include <vector>
#include "nlohmann/json.hpp"
#include "sw/redis-llm/llm_model.h"
#include "sw/redis-llm/prompt.h"
#include "sw/redis-llm/redismodule.h"
#include "sw/redis-llm/utils.h"
#include "sw/redis-llm/vector_store.h"

namespace sw::redis::llm {

struct ChatHistoryOptions {
    ChatHistoryOptions() = default;

    explicit ChatHistoryOptions(const nlohmann::json &conf);

    // Summarize every n messages. 0 means do not enable summary.
    uint32_t summary_cnt = 20;

    // Use the nearest n summaries as context.
    uint32_t summary_ctx_cnt = 1;

    std::string summary_prompt = R"(Give a concise and comprehensive summary of the given conversation (in JSON format). The summary should capture the main points and supporting details.
Conversation: """
{{conversation}}
"""
Summary: )";

    // Use latest n messages as context.
    uint32_t msg_ctx_cnt = 10;

    std::string ai_role = "assistant";

    LlmInfo llm;

    std::string store_type = "hnsw";

    nlohmann::json store_params = nlohmann::json::object();
};

class ChatHistory {
public:
    explicit ChatHistory(const ChatHistoryOptions &opts);

    std::tuple<uint64_t, std::string, Vector> add(LlmModel &model, VectorStore &store, const std::string_view &role, const std::string_view &message);

    std::pair<std::string, nlohmann::json> history(LlmModel &model, VectorStore &store, const std::string_view &input);

private:
    using Msg = std::pair<std::string, std::string>;

    std::tuple<uint64_t, std::string, Vector> _summarize_if_needed(LlmModel &model, VectorStore &store, const Msg &msg);

    std::tuple<uint64_t, std::string, Vector> _summarize(LlmModel &model, VectorStore &store, const std::vector<Msg> &msgs);

    nlohmann::json _get_latest_msgs();

    std::string _get_history_summary(LlmModel &model, VectorStore &store, const std::string_view &input, int k);

    ChatHistoryOptions _opts;

    Prompt _summary_prompt;

    std::vector<Msg> _msgs_to_be_summarize;

    std::deque<Msg> _latest_msgs;
};

}

#endif // end SEWENEW_REDIS_LLM_CHAT_HISTORY_H
