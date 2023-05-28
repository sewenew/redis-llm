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

#include "sw/redis-llm/application.h"
#include "sw/redis-llm/redis_llm.h"

namespace sw::redis::llm {

Application::Application(const nlohmann::json &llm_config,
        const nlohmann::json &embedding_config,
        const nlohmann::json &vector_store_config) {
    auto &llm = RedisLlm::instance();

    _llm = llm.llm_factory().create(llm_config);

    if (!embedding_config.empty()) {
        _embedding = llm.embedding_factory().create(embedding_config);
    }

    _vector_store = std::make_unique<VectorStore>(vector_store_config);
}

Application::Application(const nlohmann::json &llm_config,
        const nlohmann::json &embedding_config,
        const nlohmann::json &vector_store_config,
        std::unordered_map<uint64_t, std::pair<std::string, Vector>> data_store) :
    Application(llm_config, embedding_config, vector_store_config) {
    for (auto &[id, data] : data_store) {
        _vector_store->add(id, data.second);
        _data_store.emplace(id, data.first);
    }
}

std::string Application::ask(const std::string_view &question, bool without_private_data) {
    if (without_private_data) {
        return _llm->predict(question);
    }

    return _ask(question);
}

nlohmann::json Application::conf() const {
    nlohmann::json config;
    config["llm"] = _llm->conf();

    if (_embedding) {
        config["embedding"] = _embedding->conf();
    }

    config["vector_store"] = _vector_store->conf();

    return config;
}

void Application::add(uint64_t id, const std::string_view &data) {
    Vector embedding;
    if (_embedding) {
        embedding = _embedding->embedding(data);
    } else {
        embedding = _llm->embedding(data);
    }

    _vector_store->add(id, embedding);

    _data_store[id] = data;
}

void Application::add(uint64_t id, const std::string_view &data, const Vector &embedding) {
    _vector_store->add(id, embedding);

    _data_store[id] = data;
}

bool Application::rem(uint64_t id) {
    auto iter = _data_store.find(id);
    if (iter == _data_store.end()) {
        return false;
    }

    _data_store.erase(iter);

    _vector_store->rem(id);

    return true;
}

std::optional<std::string> Application::get(uint64_t id) {
    auto iter = _data_store.find(id);
    if (iter != _data_store.end()) {
        return iter->second;
    }

    return std::nullopt;
}

std::optional<Vector> Application::embedding(uint64_t id) {
    return _vector_store->get(id);
}

std::string Application::_ask(const std::string_view &question) {
    auto context = _search_private_data(question);

    auto input = _build_llm_input(question, context);

    return _llm->predict(input);
}

std::vector<std::string> Application::_search_private_data(const std::string_view &question) {
    return {};
}

std::string Application::_build_llm_input(const std::string_view &question,
        const std::vector<std::string> &context) {
    return "";
}

}
