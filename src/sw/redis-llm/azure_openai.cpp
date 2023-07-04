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

#include "sw/redis-llm/azure_openai.h"
#include "sw/redis-llm/errors.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

AzureOpenAi::AzureOpenAi(const nlohmann::json &conf) :
    LlmModel("azure_openai", conf),
    _opts(_parse_options(conf)),
    _client_pool(_opts.http_opts, _opts.http_pool_opts) {}

std::string AzureOpenAi::predict(const std::string_view &input, const nlohmann::json &params) {
    try {
        // Set model and other parameters.
        auto req = _opts.chat;
        req["messages"] = _construct_msg(input);

        auto path = "/openai/deployments/" + _opts.chat_deployment_id +
            "/chat/completions?api-version=" + _opts.api_version;
        auto ans = _query(path, req);

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

std::string AzureOpenAi::chat(const std::string_view &input,
        const std::string &history_summary,
        const nlohmann::json &recent_history,
        const nlohmann::json &params) {
    /*
    try {
        if (_opts.chat.is_null()) {
            throw Error("no chat conf is specified");
        }

        // Set model and other parameters.
        auto req = _opts.chat;
        req["messages"] = _construct_msg(input);

        auto path = "/openai/deployments/" + 
        auto ans = _query(path, req);

        return ans["choices"][0]["message"]["content"].get<std::string>();
    } catch (const std::exception &e) {
        throw Error(std::string("failed to predict: ") + e.what());
    }
    */

    return "";
}

Vector AzureOpenAi::embedding(const std::string_view &input, const nlohmann::json &params) {
    try {
        auto req = _opts.embedding;
        req["input"] = input;

        auto path = "/openai/deployments/" + _opts.embedding_deployment_id +
            "/embeddings?api-version=" + _opts.api_version;
        auto ans = _query(path, req);

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

nlohmann::json AzureOpenAi::_construct_msg(const std::string_view &input) const {
    auto messages= nlohmann::json::array();

    nlohmann::json msg;
    msg["role"] = "user";
    msg["content"] = input;

    messages.push_back(std::move(msg));

    return messages;
}

nlohmann::json AzureOpenAi::_query(const std::string &path, const nlohmann::json &req) {
    SafeClient cli(_client_pool);

    auto headers = std::unordered_multimap<std::string, std::string>{{"api-key", _opts.api_key}};
    auto output = cli.client().post(path, headers, req.dump());

    return nlohmann::json::parse(output);
}

AzureOpenAi::Options AzureOpenAi::_parse_options(const nlohmann::json &conf) const {
    Options opts;
    try {
        // {"api_key": "", "resource_name" : "", "chat_deployment_id":"", "embedding_deployment_id":"", "api_version":"", "chat": {}, "embedding": {}, "http":{"socket_timeout":"5s","connect_timeout":"5s", "enable_certificate_verification":false, "proxy_host" :"", "proxy_port":0, "pool" : {"size":3, "wait_timeout":"0s", "connection_lifetime":"0s"}}}
        opts.api_key = conf.at("api_key").get<std::string>();
        opts.resource_name = conf.at("resource_name").get<std::string>();
        opts.chat_deployment_id = conf.at("chat_deployment_id").get<std::string>();
        opts.embedding_deployment_id = conf.at("embedding_deployment_id").get<std::string>();
        opts.api_version = conf.at("api_version").get<std::string>();

        opts.chat = conf.value<nlohmann::json>("chat", nlohmann::json{});
        opts.embedding = conf.value<nlohmann::json>("embedding", nlohmann::json{});

        std::tie(opts.http_opts, opts.http_pool_opts) = _parse_http_options(conf);

        opts.http_opts.uri = "https://" + opts.resource_name + ".openai.azure.com";
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse openai options: ") + e.what() + ":" + conf.dump());
    }

    return opts;
}

auto AzureOpenAi::_parse_http_options(const nlohmann::json &conf) const
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
