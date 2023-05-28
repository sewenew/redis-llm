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

#include "sw/redis-llm/embedding_model.h"
#include <cassert>
#include "sw/redis-llm/errors.h"
#include "sw/redis-llm/openai_embedding.h"

namespace sw::redis::llm {

std::string EmbeddingModel::type() const {
    try {
        return _conf.at("type").get<std::string>();
    } catch (const nlohmann::json::exception &) {
        throw Error("no type field");
    }

    assert(false);

    return "";
}

EmbeddingModelFactory::EmbeddingModelFactory() {
    _register("openai", std::make_unique<EmbeddingModelCreatorTpl<OpenAiEmbedding>>());
}

EmbeddingModelUPtr EmbeddingModelFactory::create(const nlohmann::json &conf) const {
    auto type = conf.at("type").get<std::string>();
    auto iter = _creators.find(type);
    if (iter == _creators.end()) {
        throw Error(std::string("unknown embedding model: ") + type);
    }

    return iter->second->create(conf);
}

void EmbeddingModelFactory::_register(const std::string &type, EmbeddingModelCreatorUPtr creator) {
    if (!_creators.emplace(type, std::move(creator)).second) {
        throw Error("duplicate embedding model creators");
    }
}

}
