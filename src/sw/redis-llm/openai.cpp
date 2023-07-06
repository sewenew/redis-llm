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
    _client_pool(_opts.http_opts, _opts.http_pool_opts) {}

std::string OpenAi::predict(const std::string_view &input, const nlohmann::json &params) {
    try {
        if (_opts.chat.is_null()) {
            throw Error("no chat conf is specified");
        }

        // Set model and other parameters.
        auto req = _opts.chat;
        req["messages"] = _construct_msg(input);

        auto ans = _query(_opts.chat_path, req);

        auto &choices = ans["choices"];
        if (!choices.is_array() || choices.empty()) {
            throw Error("invalid chat choices");
        }

        auto &content = choices[0]["message"]["content"];
        if (!content.is_string()) {
            throw Error("invalid chat choices");
        }

        return content.get<std::string>();
    } catch (const std::exception &e) {
        throw Error(std::string("failed to predict with openai: ") + e.what());
    }

    return "";
}

std::string OpenAi::chat(const std::string_view &input,
        const std::string &system_msg,
        const nlohmann::json &recent_history,
        const nlohmann::json &params) {
    try {
        if (_opts.chat.is_null()) {
            throw Error("no chat conf is specified");
        }

        // Set model and other parameters.
        auto req = _opts.chat;

        req["messages"] = _construct_msg(input, system_msg, recent_history);

        auto ans = _query(_opts.chat_path, req);

        auto &choices = ans["choices"];
        if (!choices.is_array() || choices.empty()) {
            throw Error("invalid chat choices");
        }

        auto &content = choices[0]["message"]["content"];
        if (!content.is_string()) {
            throw Error("invalid chat choices");
        }

        return content.get<std::string>();
    } catch (const std::exception &e) {
        throw Error(std::string("failed to predict: ") + e.what());
    }

    return "";
}

Vector OpenAi::embedding(const std::string_view &input, const nlohmann::json &params) {
    try {
        if (_opts.embedding.is_null()) {
            throw Error("no embedding config is specified");
        }

        auto req = _opts.embedding;
        req["input"] = input;

        auto ans = _query(_opts.embedding_path, req);

        auto &data = ans["data"];
        if (!data.is_array() || data.empty()) {
            throw Error("invalid embedding response");
        }

        auto &embedding = data[0]["embedding"];
        if (!embedding.is_array()) {
            throw Error("invalid embedding response");
        }

        return embedding.get<Vector>();
    } catch (const std::exception &e) {
        throw Error(std::string("failed to request embedding: ") + e.what());
    }

    // Never reach here.
    return {};
}

nlohmann::json OpenAi::_construct_msg(const std::string_view &input,
        std::string system_info,
        nlohmann::json recent_history) const {
    nlohmann::json msgs;

    if (!system_info.empty()) {
        nlohmann::json system_msg;
        system_msg["role"] = "system";
        system_msg["content"] = std::move(system_info);
        msgs.push_back(std::move(system_msg));
    }

    for (auto &ele : recent_history) {
        msgs.push_back(std::move(ele));
    }

    nlohmann::json msg;
    msg["role"] = "user";
    msg["content"] = input;
    msgs.push_back(std::move(msg));

    return msgs;
}

nlohmann::json OpenAi::_query(const std::string &path, const nlohmann::json &req) {
    SafeClient cli(_client_pool);
    auto output = cli.client().post(path, req.dump());

    return nlohmann::json::parse(output);
}

OpenAi::Options OpenAi::_parse_options(const nlohmann::json &conf) const {
    Options opts;
    try {
        // {"api_key": "", "chat": {"chat_path":"", "model": ""}, "embedding": {"embedding_path":"", "model":""}, "http":{"socket_timeout":"5s","connect_timeout":"5s", "enable_certificate_verification":false, "pool" : {"size":3, "wait_timeout":"0s", "connection_lifetime":"0s"}}}
        opts.api_key = conf.at("api_key").get<std::string>();
        opts.chat = conf.value<nlohmann::json>("chat", nlohmann::json{});
        opts.chat_path = conf.value<std::string>("chat_path", "/v1/chat/completions");
        opts.embedding = conf.value<nlohmann::json>("embedding", nlohmann::json{});
        opts.embedding_path = conf.value<std::string>("embedding_path", "/v1/embeddings");
        if (opts.chat.find("model") == opts.chat.end()) {
            opts.chat["model"] = "gpt-3.5-turbo";
        }
        if (opts.embedding.find("model") == opts.embedding.end()) {
            opts.embedding["model"] = "text-embedding-ada-002";
        }

        std::tie(opts.http_opts, opts.http_pool_opts) = _parse_http_options(conf);

        opts.http_opts.uri = "https://api.openai.com";
        opts.http_opts.bearer_token = opts.api_key;
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse openai options: ") + e.what() + ":" + conf.dump());
    }

    return opts;
}

auto OpenAi::_parse_http_options(const nlohmann::json &conf) const
    -> std::pair<HttpClientOptions, HttpClientPoolOptions> {
    auto iter = conf.find("http");
    if (iter == conf.end()) {
        return {};
    }
    const auto &http_conf = iter.value();

    auto http_opts = HttpClientOptions(http_conf);

    HttpClientPoolOptions pool_opts;
    iter = http_conf.find("pool");
    if (iter != http_conf.end()) {
        pool_opts = HttpClientPoolOptions(iter.value());
    }

    return std::make_pair(std::move(http_opts), std::move(pool_opts));
}

}
