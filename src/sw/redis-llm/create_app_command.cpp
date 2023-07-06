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
    auto res = _create(ctx, argv, argc);

    RedisModule_ReplyWithLongLong(ctx, res);

    RedisModule_ReplicateVerbatim(ctx);
}

int CreateAppCommand::_create(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto &llm = RedisLlm::instance();

    auto args = _parse_args(argv, argc);

    auto key = api::create_key(ctx, args.key_name, llm.app_type(), args.opt);
    if (!key) {
        return 0;
    }

    auto app = llm.create_application(type(), args.llm, args.params);
    if (RedisModule_ModuleTypeSetValue(key.get(), llm.app_type(), app.get()) != REDISMODULE_OK) {
        llm.unregister_object(app);
        throw Error("failed to create APP: " + type());
    }

    return 1;
}

CreateAppCommand::Args CreateAppCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 2) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    auto idx = 2;
    while (idx < argc) {
        auto opt = util::to_sv(argv[idx]);
        if (util::str_case_equal(opt, "--NX")) {
            args.opt = api::CreateOption::NX;
        } else if (util::str_case_equal(opt, "--XX")) {
            args.opt = api::CreateOption::XX;
        } else if (util::str_case_equal(opt, "--LLM")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.llm = LlmInfo(util::to_sv(argv[idx]));
        } else if (util::str_case_equal(opt, "--PARAMS")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.params = util::to_json(argv[idx]);
        } else if (util::str_case_equal(opt, "--PROMPT")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.params["prompt"] = util::to_string(argv[idx]);
        } else {
            break;
        }

        ++idx;
    }

    if (idx < argc) {
        throw WrongArityError();
    }

    return args;
}

}
