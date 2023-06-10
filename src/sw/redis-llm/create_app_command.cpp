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

#include "sw/redis-llm/create_app_command.h"
#include "sw/redis-llm/redis_llm.h"

namespace sw::redis::llm {

void CreateAppCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    auto &llm = RedisLlm::instance();
    nlohmann::json params;
    if (!args.prompt.empty()) {
        params["prompt"] = args.prompt;
    }
    auto app = llm.app_factory().create(args.type, args.llm, params);
    if (RedisModule_ModuleTypeSetValue(&_key, llm.app_type(), app.get()) != REDISMODULE_OK) {
        throw Error("failed to create APP");
    }

    app.release();
}

CreateAppCommand::Args CreateAppCommand::_parse_args(RedisModuleString **argv, int argc) const {
    Args args;
    auto idx = 0;
    while (idx < argc) {
        auto opt = util::to_sv(argv[idx]);
        if (util::str_case_equal(opt, "--TYPE")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.type = util::to_sv(argv[idx]);
        } else if (util::str_case_equal(opt, "--LLM")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.llm = _parse_llm(util::to_sv(argv[idx]));
        } else if (util::str_case_equal(opt, "--PROMPT")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.prompt = util::to_string(argv[idx]);
        } else {
            break;
        }

        ++idx;
    }

    return args;
}

nlohmann::json CreateAppCommand::_parse_llm(const std::string_view &opt) const {
    nlohmann::json llm;
    try {
        if (!opt.empty() && opt.front() == '{') {
            llm = nlohmann::json::parse(opt.begin(), opt.end());
        } else {
            llm["key"] = std::string(opt);
        }

        if (llm.find("key") == llm.end()) {
            throw Error("no LLM key is specified");
        }

        if (llm.find("params") == llm.end()) {
            llm["params"] = nlohmann::json::object();
        }
    } catch (const nlohmann::json::exception &e) {
        throw Error(std::string("failed to parse llm parameter: ") + e.what());
    }

    return llm;
}

}
