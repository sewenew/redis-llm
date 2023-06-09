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

#include "sw/redis-llm/size_command.h"
#include "sw/redis-llm/llm_model.h"
#include "sw/redis-llm/module_api.h"
#include "sw/redis-llm/redis_llm.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

void SizeCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto size = _size(ctx, argv, argc);

    RedisModule_ReplyWithLongLong(ctx, size);
}

SizeCommand::Args SizeCommand::_parse_args(RedisModuleString **argv, int argc) const {
    assert(argv != nullptr);

    if (argc != 2) {
        throw WrongArityError();
    }

    Args args;
    args.key_name = argv[1];

    return args;
}

uint64_t SizeCommand::_size(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto args = _parse_args(argv, argc);

    auto *store = api::get_value_by_key<VectorStore>(ctx, args.key_name,
            RedisLlm::instance().vector_store_type());
    if (store == nullptr) {
        return 0;
    }

    return store->data_store().size();
}

}
