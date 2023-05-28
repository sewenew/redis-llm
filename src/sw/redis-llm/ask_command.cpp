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

#include "sw/redis-llm/ask_command.h"
#include "sw/redis-llm/application.h"
#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/redis_llm.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

void AskCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto ans = _ask(ctx, argv, argc);

    RedisModule_ReplyWithStringBuffer(ctx, ans.data(), ans.size());
}

std::string AskCommand::_ask(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    auto key = api::open_key(ctx, args.key_name, api::KeyMode::READONLY);
    assert(key);

    auto &llm = RedisLlm::instance();
    if (!api::key_exists(key.get(), llm.type())) {
        throw Error("key does not exist, call LLM.CREATE first");
    }

    auto *app = api::get_value_by_key<Application>(*key);
    assert(app != nullptr);

    return app->ask(args.question, args.no_private_data);
}

AskCommand::Args AskCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 3) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    auto idx = 2;
    while (idx < argc) {
        auto opt = util::to_sv(argv[idx]);
        if (util::str_case_equal(opt, "--NOPRIVATEDATA")) {
            args.no_private_data = true;
        } else {
            break;
        }

        ++idx;
    }

    args.question = util::to_sv(argv[idx]);

    return args;
}

}
