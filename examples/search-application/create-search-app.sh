#!/usr/bin/sh

#**************************************************************************
#  Copyright (c) 2023 sewenew
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
# *************************************************************************

redis_cli=redis-cli

if [ $# != 6 ]
then
    echo "Usage: create-search-app.sh embedding-file data-path store-key llm-key openai-api-key search-key"
    exit -1
fi

embedding_file=$1
data_path=$2
store_key=$3
llm_key=$4
openai_api_key=$5
search_key=$6

$redis_cli llm.create-llm "$llm_key" --params "{\"api_key\":\"$openai_api_key\"}"
$redis_cli llm.create-vector-store "$store_key" --llm "$llm_key"

while read line; do
    key=$(echo "$line" | cut -f1)
    embedding=$(echo "$line" | cut -f2)
    path="$data_path/$key"
    cat $path | $redis_cli -x llm.add "$store_key" --embedding "$embedding"
done <"$embedding_file"

$redis_cli llm.create-search "$search_key" --llm "$llm_key" --vector-store "$store_key"
