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

#include "sw/redis-llm/rem_command.h"
#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/redis_llm.h"
#include "sw/redis-llm/vector_store.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

void RemCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto res = _rem(ctx, argv, argc);

    RedisModule_ReplyWithLongLong(ctx, res);

    RedisModule_ReplicateVerbatim(ctx);
}

int RemCommand::_rem(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    auto key = api::open_key(ctx, args.key_name, api::KeyMode::WRITEONLY);
    assert(key);

    auto &llm = RedisLlm::instance();
    if (!api::key_exists(key.get(), llm.vector_store_type())) {
        return 0;
    }

    auto *store = api::get_value_by_key<VectorStore>(*key);
    if (store->rem(args.id)) {
        return 1;
    } else {
        return 0;
    }
}

RemCommand::Args RemCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc != 3) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];
    try {
        args.id = std::stoul(std::string(util::to_sv(argv[2])));
    } catch (const std::exception &e) {
        throw Error("invalid id");
    }

    return args;
}

}
