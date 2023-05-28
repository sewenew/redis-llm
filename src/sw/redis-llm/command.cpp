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

#include "sw/redis-llm/command.h"
#include "sw/redis-llm/add_command.h"
#include "sw/redis-llm/ask_command.h"
#include "sw/redis-llm/create_command.h"
#include "sw/redis-llm/errors.h"
#include "sw/redis-llm/get_command.h"
#include "sw/redis-llm/rem_command.h"

namespace sw::redis::llm {

int Command::run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    try {
        _run(ctx, argv, argc);

        return REDISMODULE_OK;
    } catch (const WrongArityError &) {
        return RedisModule_WrongArity(ctx);
    } catch (const Error &err) {
        return api::reply_with_error(ctx, err);
    }

    return REDISMODULE_ERR;
}

namespace cmd {

void create_commands(RedisModuleCtx *ctx) {
    if (RedisModule_CreateCommand(ctx,
                "LLM.CREATE",
                [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
                    CreateCommand cmd;
                    return cmd.run(ctx, argv, argc);
                },
                "write deny-oom",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("fail to create LLM.CREATE command");
    }

    if (RedisModule_CreateCommand(ctx,
                "LLM.ADD",
                [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
                    AddCommand cmd;
                    return cmd.run(ctx, argv, argc);
                },
                "write deny-oom",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("fail to create LLM.ADD command");
    }

    if (RedisModule_CreateCommand(ctx,
                "LLM.REM",
                [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
                    RemCommand cmd;
                    return cmd.run(ctx, argv, argc);
                },
                "write deny-oom",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("fail to create LLM.REM command");
    }

    if (RedisModule_CreateCommand(ctx,
                "LLM.GET",
                [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
                    GetCommand cmd;
                    return cmd.run(ctx, argv, argc);
                },
                "readonly",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("failed to create LLM.GET command");
    }

    if (RedisModule_CreateCommand(ctx,
                "LLM.ASK",
                [](RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
                    AskCommand cmd;
                    return cmd.run(ctx, argv, argc);
                },
                "readonly",
                1,
                1,
                1) == REDISMODULE_ERR) {
        throw Error("failed to create LLM.GET command");
    }
}

}

}
