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

#ifndef SEWENEW_REDIS_LLM_LLM_MODEL_H
#define SEWENEW_REDIS_LLM_LLM_MODEL_H

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "nlohmann/json.hpp"

namespace sw::redis::llm {

class LlmModel {
public:
    explicit LlmModel(const std::string &type, const nlohmann::json &conf) : _type(type), _conf(conf) {}

    virtual ~LlmModel() = default;

    virtual std::vector<float> embedding(const std::string_view &input) = 0;

    virtual std::string predict(const std::string_view &input) = 0;

    const std::string& type() const {
        return _type;
    }

    const nlohmann::json& conf() const {
        return _conf;
    }

private:
    std::string _type;

    nlohmann::json _conf;
};

using LlmModelUPtr = std::unique_ptr<LlmModel>;

class LlmModelCreator {
public:
    virtual ~LlmModelCreator() = default;

    virtual LlmModelUPtr create(const std::string &type, const nlohmann::json &conf) const = 0;
};

using LlmModelCreatorUPtr = std::unique_ptr<LlmModelCreator>;

template <typename T>
class LlmModelCreatorTpl : public LlmModelCreator {
public:
    virtual LlmModelUPtr create(const std::string &type, const nlohmann::json &conf) const {
        return std::make_unique<T>(conf);
    }
};

class LlmModelFactory {
public:
    LlmModelFactory();

    LlmModelUPtr create(const std::string &type, const nlohmann::json &conf) const;

private:
    void _register(const std::string &type, LlmModelCreatorUPtr creator);

    std::unordered_map<std::string, LlmModelCreatorUPtr> _creators;
};

}

#endif // end SEWENEW_REDIS_LLM_LLM_MODEL_H
