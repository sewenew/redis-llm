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

#include "sw/redis-llm/create_search_command.h"
#include "sw/redis-llm/redis_llm.h"

namespace sw::redis::llm {

void CreateSearchCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    auto &llm = RedisLlm::instance();
    auto app = llm.app_factory().create("search", args.llm, args.params);
    if (RedisModule_ModuleTypeSetValue(&_key, llm.app_type(), app.get()) != REDISMODULE_OK) {
        throw Error("failed to create APP");
    }

    app.release();
}

CreateSearchCommand::Args CreateSearchCommand::_parse_args(RedisModuleString **argv, int argc) const {
    Args args;
    auto idx = 0;
    while (idx < argc) {
        auto opt = util::to_sv(argv[idx]);
        if (util::str_case_equal(opt, "--LLM")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.llm = LlmInfo(util::to_sv(argv[idx]));
        } else if (util::str_case_equal(opt, "--VECTOR_STORE")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.params["vector_store"] = util::to_string(argv[idx]);
        } else if (util::str_case_equal(opt, "--PROMPT")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.params["prompt"] = util::to_string(argv[idx]);
        } else if (util::str_case_equal(opt, "--K")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            try {
                args.params["k"] = std::stoul(util::to_string(argv[idx]));
            } catch (const std::exception &e) {
                throw Error(std::string("invalid k") + e.what());
            }
        } else {
            break;
        }

        ++idx;
    }

    if (args.params.find("vector_store") == args.params.end()) {
        throw Error("no vector store is specified");
    }

    return args;
}

}
