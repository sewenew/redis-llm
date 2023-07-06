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
    _chat_history(_create_chat_history(conf.value<nlohmann::json>("history", {}))) {}

std::string ChatApplication::run(RedisModuleBlockedClient *blocked_client, LlmModel &model, const nlohmann::json &context, const std::string_view &input, bool verbose) {
    /*
    if (context.is_null()) {
        throw Error("no context is specified");
    }
    */

    std::lock_guard<std::mutex> lock(_mtx);

    auto [summary, recent_history] = _chat_history.history(model, input);

    nlohmann::json vars;
    vars["history"] = summary;
    auto system_msg = _system_prompt.render(vars);

    auto reply = model.chat(input, system_msg, recent_history, {});

    _chat_history.add(model, "user", input);
    _chat_history.add(model, "assistant", reply);

    return reply;
}

ChatHistory ChatApplication::_create_chat_history(const nlohmann::json &conf) const {
    ChatHistoryOptions opts(conf);
    opts.llm = llm();

    return ChatHistory(opts);
}

}
