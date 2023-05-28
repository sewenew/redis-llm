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

#include "sw/redis-llm/openai.h"
#include "sw/redis-llm/errors.h"

namespace sw::redis::llm {

OpenAi::OpenAi(const nlohmann::json &conf) :
    LlmModel(conf),
    _opts(_parse_options(conf)),
    _cli(_make_client(_opts)) {}

std::string OpenAi::predict(const std::string_view &input) {
    nlohmann::json req;
    req["model"] = _opts.model;
    req["messages"] = nlohmann::json::array();
    nlohmann::json msg;
    msg["role"] = "user";
    msg["content"] = input;
    req["messages"].push_back(std::move(msg));
    auto res = _cli->Post("/v1/chat/completions", req.dump(), "application/json");
    if (!res || res->status != 200) {
        throw Error("failed to request openai");
    }

    auto ans = nlohmann::json::parse(res->body);
    return ans["choices"][0]["message"]["content"].get<std::string>();
}

std::vector<float> OpenAi::embedding(const std::string_view &input) {
    nlohmann::json req;
    req["input"] = input;
    req["model"] = _opts.model;
    auto res = _cli->Post("/v1/embeddings", req.dump(), "application/json");
    if (!res || res->status != 200) {
        throw Error("failed to request openai embedding");
    }

    auto ans = nlohmann::json::parse(res->body);
    return ans["data"][0]["embedding"].get<std::vector<float>>();
}

OpenAi::Options OpenAi::_parse_options(const nlohmann::json &conf) const {
    Options opts;
    try {
        opts.api_key = conf.at("api_key").get<std::string>();
        opts.model = conf.at("model").get<std::string>();
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse openai options: ") + e.what());
    }

    return opts;
}

std::unique_ptr<httplib::Client> OpenAi::_make_client(const Options &opts) const {
    auto cli = std::make_unique<httplib::Client>("https://api.openai.com");
    cli->set_follow_location(true);
    cli->set_bearer_token_auth(opts.api_key);

    return cli;
}

}
