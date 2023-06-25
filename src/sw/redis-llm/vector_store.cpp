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

#include "sw/redis-llm/vector_store.h"
#include "sw/redis-llm/errors.h"
#include "sw/redis-llm/hnsw.h"

namespace sw::redis::llm {

uint64_t VectorStore::add(uint64_t id, const std::string_view &data, const Vector &embedding) {
    if (embedding.empty()) {
        throw Error("invalid embedding: size is 0");
    }

    if (_dim == 0) {
        // Use the first item's dimension as the dimension of the vector store.
        _dim = embedding.size();

        _lazily_init(_dim);
    }

    if (_dim != embedding.size()) {
        throw Error("vector dimension does not match");
    }

    _add(id, embedding);

    _data_store[id] = data;

    return id;
}

uint64_t VectorStore::add(const std::string_view &data, const Vector &embedding) {
    auto id = _auto_gen_id();
    return add(id, data, embedding);
}

bool VectorStore::rem(uint64_t id) {
    auto iter = _data_store.find(id);
    if (iter == _data_store.end()) {
        return false;
    }

    _rem(id);

    _data_store.erase(iter);

    return true;
}

std::optional<Vector> VectorStore::get(uint64_t id) {
    if (_data_store.empty()) {
        return std::nullopt;
    }

    return _get(id);
}

std::optional<std::string> VectorStore::data(uint64_t id) {
    auto iter = _data_store.find(id);
    if (iter == _data_store.end()) {
        return std::nullopt;
    }

    return iter->second;
}

std::vector<std::pair<uint64_t, float>> VectorStore::knn(const Vector &query, std::size_t k) {
    if (_data_store.empty()) {
        return {};
    }

    return _knn(query, k);
}

uint64_t VectorStore::_auto_gen_id() {
    return ++_id_idx;
}

std::size_t VectorStore::_dimension() const {
    auto iter = _conf.find("dim");
    if (iter == _conf.end()) {
        return 0;
    }

    const auto &dim = iter.value();
    if (!dim.is_number_unsigned()) {
        throw Error("invalid dim");
    }

    return dim.get<std::size_t>();
}

VectorStoreFactory::VectorStoreFactory() {
    _register("hnsw", std::make_unique<VectorStoreCreatorTpl<Hnsw>>());
}

VectorStoreSPtr VectorStoreFactory::create(const std::string &type,
        const nlohmann::json &conf, const LlmInfo &llm) const {
    auto iter = _creators.find(type);
    if (iter == _creators.end()) {
        throw Error(std::string("unknown vector store: ") + type);
    }

    return iter->second->create(conf, llm);
}

void VectorStoreFactory::_register(const std::string &type, VectorStoreCreatorUPtr creator) {
    if (!_creators.emplace(type, std::move(creator)).second) {
        throw Error("duplicate vector store creators");
    }
}

}
