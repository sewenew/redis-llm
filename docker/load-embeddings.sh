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

if [ $# != 5 ]
then
    echo "Usage: load-embeddings.sh embedding-file data-path store-key openai-key openai_api_key"
    exit -1
fi

embedding_file=$1
data_path=$2
store_key=$3
openai_key=$4
openai_api_key=$5

redis-cli llm.create llm "$openai_key" --params "{\"api_key\":\"$openai_api_key\"}"
redis-cli llm.create vector_store "$store_key" --llm "$openai_key"

while read line; do
    key=$(echo "$line" | cut -f1)
    embedding=$(echo "$line" | cut -f2)
    path="$data_path/$key"
    cat $path | redis-cli -x llm.add "$store_key" --embedding "$embedding"
done <"$embedding_file"
