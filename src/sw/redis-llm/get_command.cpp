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

#include "sw/redis-llm/get_command.h"
#include "sw/redis-llm/redis_llm.h"
#include "sw/redis-llm/vector_store.h"
#include "sw/redis-llm/utils.h"

namespace sw::redis::llm {

void GetCommand::_run(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const {
    auto res = _get(ctx, argv, argc);
    if (!res) {
        RedisModule_ReplyWithNull(ctx);
    } else {
        auto &[data, embedding] = *res;

        RedisModule_ReplyWithArray(ctx, 2);

        RedisModule_ReplyWithStringBuffer(ctx, data.data(), data.size());
        RedisModule_ReplyWithStringBuffer(ctx, embedding.data(), embedding.size());
    }
}

auto GetCommand::_get(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) const ->
    std::optional<std::pair<std::string, std::string>> {
    auto args = _parse_args(argv, argc);

    auto *store = api::get_value_by_key<VectorStore>(ctx, args.key_name,
            RedisLlm::instance().vector_store_type());
    if (store == nullptr) {
        return std::nullopt;
    }

    auto data = store->data(args.id);
    if (!data) {
        return std::nullopt;
    }

    auto embedding = store->get(args.id);
    if (!embedding) {
        return std::nullopt;
    }

    auto embedding_str = util::dump_embedding(*embedding);

    return std::make_pair(std::move(*data), std::move(embedding_str));
}

GetCommand::Args GetCommand::_parse_args(RedisModuleString **argv, int argc) const {
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
