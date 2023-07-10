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
    auto args = _parse_args(argv, argc);

    auto &llm = RedisLlm::instance();
    auto *app = api::get_value_by_key<Application>(ctx, args.key_name, llm.app_type());
    if (app == nullptr) {
        throw Error("Application does not exist");
    }

    auto *model = api::get_value_by_key<LlmModel>(ctx, app->llm().key, llm.llm_type());
    if (model == nullptr) {
        throw Error("LLM model not exist");
    }

    auto application = std::static_pointer_cast<Application>(app->shared_from_this());
    auto llm_model = std::static_pointer_cast<LlmModel>(model->shared_from_this());
    auto *blocked_client = RedisModule_BlockClient(ctx, _reply_func, _timeout_func, _free_func, args.timeout.count());
    try {
        llm.worker_pool().enqueue(&RunCommand::_run_impl, this, blocked_client, args, application, llm_model);
    } catch (const Error &err) {
        RedisModule_AbortBlock(blocked_client);

        api::reply_with_error(ctx, err);
    }
}

void RunCommand::_run_impl(RedisModuleBlockedClient *blocked_client,
        const Args &args, const ApplicationSPtr &app, const LlmModelSPtr &model) const {
    assert(blocked_client != nullptr && app && model);

    auto result = std::make_unique<AsyncResult>();

    try {
        nlohmann::json context;
        if (!args.vars.is_null()) {
            context["vars"] = args.vars;
        }

        result->output = app->run(blocked_client, *model, context, args.input, args.verbose);
    } catch (const Error &) {
        result->err = std::current_exception();
    }

    RedisModule_UnblockClient(blocked_client, result.release());
}

RunCommand::Args RunCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc < 2) {
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
        ++idx;
    }

    if (idx != argc) {
        throw WrongArityError();
    }

    return args;
}

int RunCommand::_reply_func(RedisModuleCtx *ctx, RedisModuleString ** /*argv*/, int /*argc*/) {
    auto *res = static_cast<AsyncResult *>(RedisModule_GetBlockedClientPrivateData(ctx));
    assert(res != nullptr);

    if (res->err) {
        try {
            std::rethrow_exception(res->err);
        } catch (const Error &e) {
            api::reply_with_error(ctx, e);
        }
    } else {
        RedisModule_ReplyWithStringBuffer(ctx, res->output.data(), res->output.size());
    }

    return REDISMODULE_OK;
}

int RunCommand::_timeout_func(RedisModuleCtx *ctx, RedisModuleString ** /*argv*/, int /*argc*/) {
    return RedisModule_ReplyWithNull(ctx);
}

void RunCommand::_free_func(RedisModuleCtx * /*ctx*/, void *privdata) {
    auto *result = static_cast<AsyncResult *>(privdata);
    delete result;
}

}
