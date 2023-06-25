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
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

OpenAi::OpenAi(const nlohmann::json &conf) :
    LlmModel("openai", conf),
    _opts(_parse_options(conf)),
    _cli(_make_client(_opts)) {}

std::string OpenAi::predict(const std::string_view &input, const nlohmann::json &params) {
    try {
        if (_opts.chat.is_null()) {
            throw Error("no chat conf is specified");
        }

        // Set model and other parameters.
        auto req = _opts.chat;
        req["messages"] = _construct_msg(input);

        auto ans = _query(_opts.chat_uri, req);

        return ans["choices"][0]["message"]["content"].get<std::string>();
    } catch (const std::exception &e) {
        throw Error(std::string("failed to predict: ") + e.what());
    }

    return "";
}

std::string OpenAi::chat(const std::string_view &input, const nlohmann::json &params) {
    /*
    try {
        if (_opts.chat.is_null()) {
            throw Error("no chat conf is specified");
        }

        // Set model and other parameters.
        auto req = _opts.chat;
        req["messages"] = _construct_msg(input);

        auto ans = _query(_opts.chat_uri, req);

        return ans["choices"][0]["message"]["content"].get<std::string>();
    } catch (const std::exception &e) {
        throw Error(std::string("failed to predict: ") + e.what());
    }
    */

    return "";
}

Vector OpenAi::embedding(const std::string_view &input, const nlohmann::json &params) {
    try {
        if (_opts.embedding.is_null()) {
            throw Error("no embedding config is specified");
        }

        auto req = _opts.embedding;
        req["input"] = input;

        auto ans = _query(_opts.embedding_uri, req);

        return ans["data"][0]["embedding"].get<Vector>();
    } catch (const std::exception &e) {
        throw Error(std::string("failed to request embedding: ") + e.what());
    }

    // Never reach here.
    return {};
}

nlohmann::json OpenAi::_construct_msg(const std::string_view &input) const {
    auto messages= nlohmann::json::array();

    nlohmann::json msg;
    msg["role"] = "user";
    msg["content"] = input;

    messages.push_back(std::move(msg));

    return messages;
}

nlohmann::json OpenAi::_query(const std::string &path, const nlohmann::json &req) {
    auto res = _cli->Post(path, req.dump(), "application/json");
    if (!res) {
        throw Error("failed to request openai: " + httplib::to_string(res.error()));
    }

    if (res->status != 200) {
        throw Error("failed to request openai: " + std::to_string(res->status) + ", " + res->reason);
    }

    return nlohmann::json::parse(res->body);
}

OpenAi::Options OpenAi::_parse_options(const nlohmann::json &conf) const {
    Options opts;
    try {
        opts.api_key = conf.at("api_key").get<std::string>();
        opts.chat = conf.value<nlohmann::json>("chat", nlohmann::json{});
        opts.chat_uri = conf.value<std::string>("chat_uri", "/v1/chat/completions");
        opts.embedding = conf.value<nlohmann::json>("embedding", nlohmann::json{});
        opts.embedding_uri = conf.value<std::string>("embedding_uri", "/v1/embeddings");
        if (opts.chat.find("model") == opts.chat.end()) {
            opts.chat["model"] = "gpt-3.5-turbo";
        }
        if (opts.embedding.find("model") == opts.embedding.end()) {
            opts.embedding["model"] = "text-embedding-ada-002";
        }
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse openai options: ") + e.what() + ":" + conf.dump());
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
