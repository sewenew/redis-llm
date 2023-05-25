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

#ifndef SEWENEW_REDIS_LLM_EMBEDDING_MODEL_H
#define SEWENEW_REDIS_LLM_EMBEDDING_MODEL_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace sw::redis::llm {

class EmbeddingModel {
public:
    explicit EmbeddingModel(const std::string &type) : _type(type) {}

    virtual ~EmbeddingModel() = default;

    virtual std::vector<float> embedding(const std::string_view &input) = 0;

    virtual std::string serialize() = 0;

    virtual void deserialize(const std::string_view &data) = 0;

    const std::string& type() const {
        return _type;
    }

private:
    std::string _type;
};

using EmbeddingModelUPtr = std::unqiue_ptr<EmbeddingModel>;

class EmbeddingModelCreator {
public:
    virtual ~EmbeddingModelCreator() = default;

    virtual EmbeddingModelUPtr create(const std::vector<std::string_view> args) const = 0;
};

using EmbeddingModelCreatorUPtr = std::unique_ptr<EmbeddingModelCreator>;

template <typename T>
class EmbeddingModelCreatorTpl : public EmbeddingModelCreator {
public:
    virtual EmbeddingModelUPtr create(const std::vector<std::string_view> &args) const {
        return std::make_unique<T>(args);
    }
};

class EmbeddingModelFactory {
public:
    EmbeddingModelFactory();

    EmbeddingModelUPtr create(const std::string_view &type, const std::vector<std::string_view> &args) const;

private:
    void _register(const std::string &type, EmbeddingModelCreatorUPtr creator);

    std::unordered_map<std::string, EmbeddingModelCreatorUPtr> _creators;
};

}

#endif // end SEWENEW_REDIS_LLM_EMBEDDING_MODEL_H
