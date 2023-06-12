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

#include "sw/redis-llm/hnsw.h"
#include <algorithm>

namespace sw::redis::llm {

Hnsw::Hnsw(const nlohmann::json &conf, const LlmInfo &llm) :
    VectorStore("hnsw", conf, llm), _opts(_parse_options(conf)) {
    _space = std::make_unique<hnswlib::L2Space>(_opts.dim);
    _hnsw = std::make_unique<hnswlib::HierarchicalNSW<float>>(_space.get(), _opts.max_elements, _opts.m, _opts.ef_construction);
}

bool Hnsw::rem(uint64_t id) {
    auto iter = _data_store.find(id);
    if (iter == _data_store.end()) {
        return false;
    }

    try {
        _hnsw->markDelete(id);
    } catch (const std::exception &e) {
        throw Error("failed to delete: " + std::to_string(id) + ", err: " + e.what());
    }

    _data_store.erase(iter);
}

std::optional<Vector> Hnsw::get(uint64_t id) {
    try {
        return _hnsw->getDataByLabel<float>(id);
    } catch (const std::exception &e) {
        // Fall through
    }

    return std::nullopt;
}

std::optional<std::string> Hnsw::data(uint64_t id) {
    auto iter = _data_store.find(id);
    if (iter == _data_store.end()) {
        return std::nullopt;
    }

    return iter->second;
}

std::vector<std::pair<uint64_t, float>> Hnsw::knn(const Vector &query, std::size_t k) {
    std::vector<std::pair<uint64_t, float>> output;
    try {
        auto res = _hnsw->searchKnn(query.data(), k);
        while (!res.empty()) {
            auto &ele = res.top();
            output.emplace_back(ele.second, ele.first);
            res.pop();
        }
        std::reverse(output.begin(), output.end());
    } catch (const std::exception &e) {
        throw Error("failed to do knn");
    }

    return output;
}

void Hnsw::_add(uint64_t id, const std::string_view &data, const Vector &embedding) {
    if (embedding.size() != _opts.dim) {
        throw Error("vector dimension does not match");
    }

    try {
        _hnsw->addPoint(embedding.data(), id);
    } catch (const std::exception &e) {
        throw Error("failed to do set: " + std::to_string(id));
    }

    _data_store[id] = data;
}

Hnsw::Options Hnsw::_parse_options(const nlohmann::json &conf) const {
    Options opts;
    try {
        opts.dim = conf.at("dim").get<std::size_t>();
        opts.max_elements = conf.value<std::size_t>("max_elements", 10000);
        opts.m = conf.value<std::size_t>("m", 16);
        opts.ef_construction = conf.value<std::size_t>("ef_construction", 200);
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse vector store options: ") + e.what());
    }

    return opts;
}


}
