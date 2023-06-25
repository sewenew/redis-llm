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

#include "sw/redis-llm/utils.h"
#include <cassert>
#include <cctype>
#include "sw/redis-llm/errors.h"

namespace sw::redis::llm {

LlmInfo::LlmInfo(const std::string_view &info) {
    try {
        nlohmann::json llm;
        if (!info.empty() && info.front() == '{') {
            llm = nlohmann::json::parse(info.begin(), info.end());
        } else {
            llm["key"] = std::string(info);
        }

        _init(std::move(llm));
    } catch (const std::exception &e) {
        throw Error(std::string("invalid LLM info: ") + e.what());
    }
}

std::string LlmInfo::to_string() const {
    std::string info;
    try {
        nlohmann::json llm;
        llm["key"] = key;
        llm["params"] = params;

        info = llm.dump();
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("LlmInfo::to_string fail: ") + e.what());
    }

    return info;
}

void LlmInfo::_init(nlohmann::json info) {
    if (info.is_null()) {
        throw Error("null LLM info");
    }

    try {
        key = info.at("key").get<std::string>();

        params = info.value<nlohmann::json>("params", nlohmann::json::object());
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("invalid LLM info: ") + e.what());
    }
}

namespace util {

std::string_view to_sv(RedisModuleString *str) {
    if (str == nullptr) {
        throw Error("null string");
    }

    std::size_t len = 0;
    auto *data = RedisModule_StringPtrLen(str, &len);
    return {data, len};
}

std::vector<std::string_view> to_sv(RedisModuleString **argv, int argc) {
    assert(argv != nullptr && argc >= 0);

    std::vector<std::string_view> args;
    args.reserve(argc);
    for (auto idx = 0; idx < argc; ++idx) {
        args.push_back(to_sv(argv[idx]));
    }

    return args;
}

std::string to_string(RedisModuleString *str) {
    if (str == nullptr) {
        throw Error("null string");
    }

    std::size_t len = 0;
    auto *data = RedisModule_StringPtrLen(str, &len);
    return {data, len};
}

nlohmann::json to_json(RedisModuleString *str) {
    auto sv = to_sv(str);
    nlohmann::json info;
    try {
        info = nlohmann::json::parse(sv.begin(), sv.end());
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse json: ") + e.what());
    }

    return info;
}

bool str_case_equal(const std::string_view &s1, const std::string_view &s2) {
    if (s1.size() != s2.size()) {
        return false;
    }

    const auto *p1 = s1.data();
    const auto *p2 = s2.data();
    for (std::size_t idx = 0; idx != s1.size(); ++idx) {
        if (static_cast<char>(std::toupper(static_cast<unsigned char>(p1[idx])))
                != static_cast<char>(std::toupper(static_cast<unsigned char>(p2[idx])))) {
            return false;
        }
    }

    return true;
}

Vector parse_embedding(const std::string_view &opt) {
    std::vector<std::string_view> vec;
    util::split(opt, ",", std::back_inserter(vec));
    Vector embedding;
    embedding.reserve(vec.size());
    for (auto &ele : vec) {
        try {
            embedding.push_back(stof(std::string(ele)));
        } catch (const std::exception &) {
            throw Error("invalid embedding");
        }
    }

    return embedding;
}

std::string dump_embedding(const Vector &embedding) {
    std::string embedding_str;
    for (auto ele : embedding) {
        if (!embedding_str.empty()) {
            embedding_str += ",";
        }
        embedding_str += std::to_string(ele);
    }

    return embedding_str;
}

}

}
