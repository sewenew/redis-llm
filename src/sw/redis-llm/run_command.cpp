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

#include "sw/redis-llm/run_command.h"
#include <cassert>
#include "sw/redis-llm/application.h"
#include "sw/redis-llm/redis_llm.h"

namespace sw::redis::llm {

void RunCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto res = _run_impl(ctx, argv, argc);
    RedisModule_ReplyWithStringBuffer(ctx, res.data(), res.size());
}

std::string RunCommand::_run_impl(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    auto key = api::open_key(ctx, args.key_name, api::KeyMode::READONLY);
    assert(key);

    auto &llm = RedisLlm::instance();
    if (!api::key_exists(key.get(), llm.app_type())) {
        throw Error("app does not exist, call LLM.CREATE APP first");
    }

    auto *app = api::get_value_by_key<Application>(*key);
    auto *model = api::get_value_by_key<LlmModel>(ctx, app->llm().key, llm.llm_type());
    if (model == nullptr) {
        throw Error("LLM model does not exist");
    }

    nlohmann::json context;
    if (!args.vars.is_null()) {
        context["vars"] = args.vars;
    }

    return app->run(ctx, *model, context, args.input, args.verbose);
}

RunCommand::Args RunCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != 3);

    if (argc < 3) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    auto idx = 2;
    while (idx < argc) {
        auto opt = util::to_sv(argv[idx]);
        if (util::str_case_equal(opt, "--VARS")) {
            if (idx + 1 >= argc) {
                throw Error("syntax error");
            }
            ++idx;
            args.vars = util::to_json(argv[idx]);
        } else if (util::str_case_equal(opt, "--VERBOSE")) {
            args.verbose = true;
        } else {
            break;
        }

        ++idx;
    }

    if (idx < argc) {
        args.input = util::to_sv(argv[idx]);
    }

    return args;
}

}
