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
#include "sw/redis-llm/hnsw.h"

namespace sw::redis::llm {

std::size_t VectorStore::dim() const {
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

VectorStoreUPtr VectorStoreFactory::create(const std::string &type, const nlohmann::json &conf, const std::string &llm) const {
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
