# redis-llm

[English](https://github.com/sewenew/redis-llm/blob/main/README.md)

- [概览](#概览)
    - [功能](#功能)
- [安装](#安装)
    - [使用Docker运行redis-llm](#使用Docker运行redis-llm)
    - [从源码安装redis-llm](#从源码安装redis-llm)
    - [加载redis-llm](#加载redis-llm)
- [快速开始](#快速开始)
    - [redis-cli](#redis-cli)
    - [C++客户端](#c-客户端)
    - [Python客户端](#python-客户端)
- [术语](#术语)
    - [LLM](#llm)
    - [Prompt](#prompt)
    - [Vector Store](#vector-store-2)
    - [Application](#application)
- [命令](#命令)
    - [LLM.CREATE-LLM](#llmcreate-llm)
    - [LLM.CREATE-VECTOR-STORE](#llmcreate-vector-store)
    - [LLM.CREATE-APP](#llmcreate-app)
    - [LLM.CREATE-SEARCH](#llmcreate-search)
    - [LLM.CREATE-CHAT](#llmcreate-chat)
    - [LLM.ADD](#llmadd)
    - [LLM.GET](#llmget)
    - [LLM.REM](#llmrem)
    - [LLM.SIZE](#llmsize)
    - [LLM.KNN](#llmknn)
- [作者](#作者)

## 概览

redis是一个[Redis模块](https://redis.io/topics/modules-intro)，它将大语言模型（LLM）集成到了Redis中。

大语言模型非常强大，但也有它自身的局限。

- 大语言模型无法访问用户的私有数据，不能基于这些私有数据来回答用户的问题。
- 大语言模型存在token数量的限制，不能记住很长的对话历史。

为了解决这些问题，我写了redis模块，将大语言模型集成到Redis中，使得我们可以把Redis作为大语言模型的外部存储。你可以将私有数据通过redis存储到Redis中，然后让大语言模型基于这些数据来回答问题。你也可以通过redis来和大语言模型对话，redis会记录并索引你们大对话历史，使得大语言模型可以记住几乎所有的对话内容。

### 功能

- [利用Prompt（提示词）来构建大语言模型应用](#simple-application)
- [向量数据库](#vector-store)
- [针对私有数据进行提问](#search-application)
- [让大语言模型记住几乎所有的聊天记录](#chat-application)

## 安装

### 使用Docker运行redis-llm

如果你安装了docker，那么可以通过以下命令来运行redis。

```
docker run -p 6379:6379 sewenew/redis-llm:latest
```

通过以上命令，Docker会使用容器中*/usr/lib/redis/conf/redis.conf*配置来运行一个Redis实例。

Docker容器跑起来后，你可以跳转到[快速入门章节](#快速开始)，查看如何执行redis命令。

### 从源码安装redis-llm

你也可以直接从源码安装redis-llm模块。

redis-llm依赖于curl和openssl，因此你需要先安装这些依赖。

```
apt-get install libssl-dev libcurl4-openssl-dev
```

redis使用[CMAKE](https://cmake.org)编译。

```
git clone https://github.com/sewenew/redis-llm.git

cd redis-llm

mkdir compile

cd compile

cmake ..

make
```

执行完make之后，你可以在*redis-llm/compile*目录下找到*libredis-llm.so*动态库（如果是在MacOS下，那么动态库的名字是*libredis-llm.dylib*）。

### 加载redis-llm

redis-llm模块依赖于Redis 5.0的模块API，因此你需要安装Redis 5.0或者更高的版本。

修改*redis.conf*文件，加入loadmodule配置：

```
loadmodule /path/to/libredis-llm.so
```

现在你就可以启动Redis实例了：

```
redis-server /path/to/redis.conf
```

如果Redis加载模块成功，那么你可以在日志文件中看到以下消息：

```
Module 'LLM' loaded from /path/to/libredis-llm.so
```

#### 模块参数

redis-llm使用一个线程池来运行一些高耗时的任务。它会首先把这些任务提交的一个队列中，线程池中的工作线程会从队列中取出任务来执行。你可以在加载模块的时候使用以下选项来设置队列大小和线程池的大小：

- **--QUEUE_SIZE**：任务队列大小。可选。默认1000。
- **--POOL_SIZE**：线程池大小。可选。默认10。

```
loadmodule /path/to/libredis-llm.so --QUEUE_SIZE 3000 --POOL_SIZE 20
```

## 快速开始

完成[模块加载](#加载redis-llm)后，你就可以使用任何Redis客户端向Redis发送[redis-llm命令](#命令)了。

### redis-cli

以下示例使用官方的Redis客户端：redis-cli，发送命令。

**注意**：如果你发送的命令包含非英文，请在启动redis-cli的时候，使用--raw选项：`redis-cli --raw`。

列出模块信息:

```
127.0.0.1:6379> MODULE LIST
1) 1) "name"
   2) "LLM"
   3) "ver"
   4) (integer) 1
```

用OpenAI API key创建一个OpenAI类型的大语言模型。

```
127.0.0.1:6379> LLM.CREATE-LLM model --TYPE openai --PARAMS '{"api_key" : "$OPENAI_API_KEY"}'
(integer) 1
```

#### Vector Store

创建一个不带LLM支持的vector store。对于这类vector store，在插入数据时，需要显式指定数据的embedding。

```
127.0.0.1:6379> LLM.CREATE-VECTOR-STORE store-without-llm-support
(integer) 1
127.0.0.1:6379> LLM.ADD store-without-llm-support --ID 1 --EMBEDDING 1.1,2.2,3.3 'some data'
(integer) 1
127.0.0.1:6379> LLM.ADD store-without-llm-support --ID 2 --EMBEDDING 2.2,3.3,4.4 'some other data'
(integer) 2
127.0.0.1:6379> LLM.SIZE store-without-llm-support
(integer) 2
127.0.0.1:6379> LLM.KNN store-without-llm-support --K 1 --EMBEDDING 1,2,3
1) 1) (integer) 1
   2) "0.14000000059604645"
127.0.0.1:6379> LLM.GET store-without-llm-support 1
1) "some data"
2) "1.100000,2.200000,3.300000"
127.0.0.1:6379> LLM.REM store-without-llm-support 1
(integer) 1
127.0.0.1:6379> LLM.SIZE store-without-llm-support
(integer) 1
```

创建一个带LLM支持的vector store。向其中插入数据时，会自动调用LLM生成插入数据的embedding。

```
127.0.0.1:6379> LLM.CREATE-LLM model --TYPE openai --PARAMS '{"api_key" : "$OPENAI_API_KEY"}'
(integer) 1
127.0.0.1:6379> LLM.CREATE-VECTOR-STORE store --LLM model
(integer) 1
127.0.0.1:6379> LLM.ADD store 'redis-llm is a Redis module that integrates LLM (Large Language Model) with Redis'
(integer) 1
127.0.0.1:6379> LLM.KNN store 'redis-llm is a Redis module'
1) 1) (integer) 1
   2) "0.05000000059604645"
```

#### Simple Application

使用LLM模型创建一个*hello world*应用，输入一段文字来运行这个应用。

```
127.0.0.1:6379> LLM.CREATE-LLM model --TYPE openai --PARAMS '{"api_key" : "$OPENAI_API_KEY"}'
(integer) 1
127.0.0.1:6379> LLM.CREATE-APP hello-world --LLM model
(integer) 1
127.0.0.1:6379> LLM.RUN hello-world 'Say hello to LLM'
"Hello LLM! It's nice to meet you. How can I assist you today?"
```

使用一个LLM模型和一段prompt创建一个*hello world*应用（你需要事先调用LLM.CREATE-LLM创建好LLM模型）。

```
127.0.0.1:6379> LLM.CREATE-APP hello-world --LLM model --PROMPT 'Say hello to LLM'
(integer) 1
127.0.0.1:6379> LLM.RUN hello-world
"Hello LLM! It's nice to meet you. How can I assist you today?"
```

使用一个LLM模型和一个prompt模版创建一个*hello world*应用（你需要事先调用LLM.CREATE-LLM创建好LLM模型）。在运行该应用时，需要为模版所需的变量赋值。

```
127.0.0.1:6379> LLM.CREATE-APP hello-world --LLM openai --PROMPT 'Say hello to {{name}}'
(integer) 1
127.0.0.1:6379> LLM.RUN hello-world --vars '{"name":"LLM"}'
"Hello LLM! It's nice to meet you. How can I assist you today?"
```

#### Search Application

创建一个search（查询）应用，让它基于vector store中保存的私有数据来回答问题。

- 调用LLM.CREATE-LLM创建一个LLM模型
- 调用LLM.CREATE-VECTOR-STORE创建一个带LLM支持的vector store
- 调用LLM.ADD将私有数据添加到vector store中
- 调用LLM.CREATE-SEARCH，用创建好的LLM模型和vector store来创建一个search应用
- 调用LLM.RUN向该search应用提问

```
127.0.0.1:6379> LLM.CREATE-LLM model --TYPE openai --PARAMS '{"api_key" : "$OPENAI_API_KEY"}'
(integer) 1
127.0.0.1:6379> LLM.CREATE-VECTOR-STORE store --LLM model
(integer) 1
127.0.0.1:6379> LLM.ADD store 'redis-llm is an open source project written by sewenew'
(integer) 1
127.0.0.1:6379> LLM.CREATE-SEARCH search-private-data --LLM model --VECTOR-STORE store
(integer) 1
127.0.0.1:6379> LLM.RUN search-private-data 'who is the author of redis-llm'
"The author of redis-llm is sewenew."
```

[这里](https://github.com/sewenew/redis-llm/tree/main/examples/search-application)包含了基于Redis命令文档和redis-plus-plus文档构建私有数据检索的例子。

#### Chat Application

创建一个chat（聊天）应用，该应用能帮助LLM记住很长的聊天历史。

- 调用LLM.CREATE-LLM创建一个LLM模型
- 调用LLM.CREATE-VECTOR-STORE创建一个带LLM支持的vector store。chat应用会把聊天记录索引到该vector store中
- 调用LLM.CREATE-CHAT，用创建好的LLM模型和vector store来创建一个chat应用
- 调用LLM.RUN发起聊天

```
127.0.0.1:6379> LLM.CREATE-LLM model --TYPE openai --PARAMS '{"api_key" : "$OPENAI_API_KEY"}'
(integer) 1
127.0.0.1:6379> LLM.CREATE-VECTOR-STORE history --LLM model
(integer) 1
127.0.0.1:6379> LLM.CREATE-CHAT chat --LLM model --VECTOR-STORE history
(integer) 1
127.0.0.1:6379> LLM.RUN chat 'Can you recommend a C++ Redis client library for me?'
```

### C++客户端

如果你使用C++，那么你可以使用[redis-plus-plus](https://github.com/sewenew/redis-plus-plus)来发送redis命令：

```C++
```

### Python客户端

如果你使用Python，那么你可以使用[redis-py](https://github.com/redis/redis-py) to send redis命令：

```Python
```

## 术语

你可以使用redis来创建大语言模型应用程序，例如：聊天机器人。以下是一些相关的术语介绍。

### LLM

大语言模型。当前我们支持以下模型：

- [OpenAI](https://platform.openai.com/docs/api-reference)
- [Azure OpenAI](https://azure.microsoft.com/en-us/products/cognitive-services/openai-service)

如果你需要其它模型的支持，欢迎在[这个issue](https://github.com/sewenew/redis-llm/issues/2)下留言。

### Prompt

提示词。通常你输入一段提示词（prompt），LLM会返回一个相应的回答（completion）。使用一段好的提示词才能得到一个好的答案。[这里](https://help.openai.com/en/articles/6654000-best-practices-for-prompt-engineering-with-openai-api)有一些构建提示词的最佳实践可供参考。

以下命令使用一段提示词来构建一个LLM应用：

```
LLM.CREATE-APP test-application --PROMPT 'You are an expert on prompt engineering. Please give some best practices for prompt engineering.'
```

一旦你运行这个应用，LLM就会返回你一段基于你的提示词的回答，即：关于提示词工程的最佳实践。

```
LLM.RUN test-application
```

#### Prompt模版

除了硬编码的提示词外，redis也支持提示词模版。你可以构建一个包含变量的模版，给变量赋不同的值，得到不同的提示词。

以下命令使用一个提示词模版来创建一个LLM应用（注意：你需要先创建一个名为*llm*的大语言模型）。

```
LLM.CREATE-APP app-with-prompt-template --LLM llm --PROMPT 'You are an expert on {{domain}}. Please answer the following question: {{question}}'
```

*{{*和*}}*之间的部分是模版变量。以上例子包含了两个变量：*domain*和*question*。我们在运行这个应用时，可以通过对变量赋不同的值来使用不同的提示词。

```
LLM.RUN app-with-prompt-template --LLM llm --VARS '{"domain" : "LLM", "question" : "Please give an introduction on LLM."}'
```

我们可以使用*--VARS*选项来为每个变量赋值。这个选项采用JSON格式，JSON的每个key-value为对应的变量赋值。redis会将以上模版渲染为以下提示词，然后使用该提示词来向LLM发问。

```
You are an expert on LLM. Please answer the following question: Please give an introduction on LLM.
```

### Vector Store

向量数据库。向量数据库用于存储数据，以及数据对应的embedding，即：向量。embedding可以表示数据的语义，具有相似embedding的数据往往在语义上也是相似的。通常，我们采用ANN算法来搜索给定数据的K个可能的最近邻数据。

有很多ANN算法，我们目前支持以下算法：

- [HNSW](https://github.com/nmslib/hnswlib)

### Application

应用。使用redis-llm，你可以构建LLM应用。

目前我们支持以下应用：

- **Simple Application**: 基于一个Prompt或者Prompt模版构建的简单应用。你可以向它输入一个字符串（prompt），它会输出一个字符串（completion）。查看[LLM.CREATE-APP命令](#llmcreate-app)获取更详细的说明。
- **Search Application**: 你可以事先向一个vector store存储私有数据，然后通过Search Application对私有数据进行查询。查看[LLM.CREATE-SEARCH命令](#llmcreate-search)获取更详细的说明。
- **Chat Application**: 你可以使用该应用来跟LLM聊天，它会索引你们之间的聊天记录，进而帮助LLM记住你们之间的谈话。查看[LLM.CREATE-CHAT命令](#llmcreate-chat)获取更详细的说明。

## 命令

命令名字和选项名字大小写不敏感。

### LLM.CREATE-LLM

#### 语法

```
LLM.CREATE-LLM key [--NX] [--XX] [--TYPE model-type] --PARAMS '{model parameters in JSON format}'
```

**LLM.CREATE-LLM** 创建一个LLM模型，并保存到*key*中。所有的应用（application）都需要使用LLM，因此在创建应用前，你需要先创建好LLM模型。

#### 选项

- **--NX**: 当且仅当*key*不存在时，创建模型。可选。
- **--XX**: 当且仅当*key*存在时，创建模型。可选。
- **--TYPE**: 模型类型。可选。如果没有设置，默认类型为openai。
- **--PARAMS**: JSON格式的模型参数。详情请查看[LLM模型章节](#LLM模型)。

#### LLM模型

##### openai

如果你要使用OpenAI，那么可以直接模型类型为`--TYPE openai`，其模型参数如下：

```JSON
{"api_key": "required", "chat_path": "/v1/chat/completions", "chat": {"model": "gpt-3.5-turbo"}, "embedding_path": "/v1/embeddings", "embedding": {"model":"text-embedding-ada-002"}, "http":{"socket_timeout":"5s","connect_timeout":"5s", "enable_certificate_verification":false, "proxy_host": "", "proxy_port": 0, "pool" : {"size": 5, "wait_timeout":"0s", "connection_lifetime":"0s"}}}
```

以上参数都是键值对，必须字段用*required*标识，可选字段则给出了默认值。如果没有设置，那么会使用给定的默认直。例如，要使用*gpt-3.5-turbo-0301*模型，其它参数使用默认值，那么可以进行如下设置：

```
LLM.CREATE-LLM key --PARAMS '{"api_key" : "sk-your-api-key", "chat": {"model": "gpt-3.5-turbo-0301"}}'
```

如果你想为chat或者embedding API设置OpenAI的[其它参数](https://platform.openai.com/docs/api-reference/chat/create)，那么可以把这些参数设置到*chat*参数里。以下例子为chat API设置了*api_key*和*temperature*参数:

```
LLM.CREATE-LLM key --PARAMS '{"api_key" : "sk-your-api-key", "chat": {"temperature": 0.5}}'
```

##### azure openai

如果你想使用Azure OpenAI，那么可以设置模型类型为`--TYPE azure_openai`，其模型参数如下：

```JSON
{"api_key": "required", "resource_name" : "required", "chat_deployment_id": "required", "embedding_deployment_id": "required", "api_version": "required", "chat": {}, "embedding": {}, "http": {"socket_timeout":"5s", "connect_timeout":"5s", "enable_certificate_verification": false, "proxy_host": "", "proxy_port": 0, "pool" : {"size": 5, "wait_timeout":"0s", "connection_lifetime":"0s"}}}
```

所有的参数都是键值对，必选字段标识为了*required*，可选字段给出了默认值。如果没有设置，那么就使用默认值。例如，如果你想要设置超时时间（*socket_time*）为10秒，其它使用默认值，那么可以如下设置：

```
LLM.CREATE-LLM key --TYPE azure_openai --PARAMS '{"api_key" : "sk-your-api-key", "resource_name": "your-resource_name", "chat_deployment_id": "your-chat_deployment_id", "embedding_deployment_id": "your-embedding_deployment_id", "api_version" : "api-version", "http": {"socket_timeout" : "10s"}}'
```

如果你想要为chat或者embedding API设置[其它参数](https://learn.microsoft.com/en-us/azure/cognitive-services/openai/reference)，那么可以把它们设置到*chat*字段。以下例子设置了*temperature*参数：

```
LLM.CREATE-LLM key --TYPE azure_openai --PARAMS '{"api_key" : "sk-your-api-key", "resource_name": "your-resource_name", "chat_deployment_id": "your-chat_deployment_id", "embedding_deployment_id": "your-embedding_deployment_id", "api_version" : "api-version", "chat": {"temperature" : 0.5}}'
```

#### 返回值

- *Integer reply*: 如果模型创建成功，那么返回1，否则返回0（例如设置了*--NX*选项，但key已经存在了）。

#### 错误

以下情况会返回错误：

- 模型类型不支持。
- 不能用给定的参数创建模型。
- 存储在*key*处的数据不是LLM模型。

#### 示例

```
// 创建一个openai类型的LLM模型，设置api key，其它参数使用默认值
LLM.CREATE-LLM key --TYPE openai --PARAMS '{"api_key": "Your API KEY"}'

// 创建一个openai类型的LLM模型，设置api key，chat API的模型使用gpt-3.5-turbo-0301
LLM.CREATE-LLM key --TYPE openai --PARAMS '{"api_key": "Your API KEY", "chat" : {"model" : "gpt-3.5-turbo-0301"}}'

// 创建一个azure openai类型的LLM模型，设置必须参数
LLM.CREATE-LLM key --TYPE azure_openai --PARAMS '{"api_key": "Your API KEY", "resource_name": "your resource name", "chat_deployment_id": "your deployment id for chat api", "embedding_deployment_id": "your deployment id for embedding api", "api_version": "api version"}'

// 创建一个azure openai类型的LLM模型，设置必须参数，设置http代理
LLM.CREATE-LLM key --TYPE azure_openai --PARAMS '{"api_key": "Your API KEY", "resource_name": "your resource name", "chat_deployment_id": "your deployment id for chat api", "embedding_deployment_id": "your deployment id for embedding api", "api_version": "api version", "http": {"proxy_host": "http://xxx.xxx.xx", "proxy_port": 3149}}'
```

### LLM.CREATE-VECTOR-STORE

#### 语法

```
LLM.CREATE-VECTOR-STORE key [--NX] [--XX] [--TYPE vector-store-type] [--LLM llm-info] [--PARAMS '{store parameters in JSON format}']
```

**LLM.CREATE-VECTOR-STORE**创建一个vector store，并保存在*key*处。如果你想让LLM应用访问你的私有数据，你需要先创建一个vector store。

#### 选项

- **--NX**: 当且仅当*key*不存在时，创建vector store。可选。
- **--XX**: 当且仅当*key*存在时，创建模型。可选。
- **--TYPE**: vector store的类型。可选。如果没有指定，那么默认使用*hnsw*类型。
- **--LLM**: vector store使用的LLM模型存储在Redis中的key。如果使用LLM.ADD命令的时候没有指定embedding，那么redis-llm会使用该LLM模型生成embedding，然后再存储到vector store中。
- **--PARAMS**: JSON格式的vector store参数。详情请查看[vector store章节](#vector-stores)。可选。

#### Vector Stores

##### hnsw

目前我们只支持HNSW类型，这也是默认类型。当然，你也可以显式的通过`--TYPE hnsw`指定。它的参数如下：

```JSON
{"max_elements": 100000, "m": 16, "ef_construction": 200}
```

所有的参数都是键值对。必选参数使用*required*标识，可选参数给出了默认值。

- *max_elements*: vector store能存储的最大数量。

**NOTE**: vector store使用第一条插入到vector store的embedding的维度作为vector store的维度。

#### 返回值

- *Integer reply*: 如果创建vector store成功，返回1，否则返回0（例如设置了*--NX*选项，但key已经存在了）。

#### 错误

以下情况会返回错误：

- vector store的类型不支持。
- 无法使用给定的参数创建vector store。
- 存储在*key*处的数据不是vector store类型。

#### 示例

以下示例创建一个带LLM支持的vector store，即：指定了--LLM选项。你可以在插入数据的时如果没有指定数据的embedding，那么redis-llm会使用指定的LLM自动生成数据的embedding，然后插入。详情请查看[LLM.ADD命令](#llmadd-command)。

```
// 创建一个默认类型（hnsw）的vector store
LLM.CREATE-VECTOR-STORE key --LLM llm-key

// 显式指定vector store的类型为hnsw
LLM.CREATE-VECTOR-STORE key --TYPE hnsw --LLM llm-key

// 创建vector store，并指定一些参数
LLM.CREATE-VECTOR-STORE key --LLM llm-key --PARAMS '{"max_elements": 20000}'
```

在某些场景下，我们已经获取了数据的embedding，不再需要通过LLM模型生成了。这种情况下，在创建的时候，不需要指定--LLM参数。

```
// 创建一个默认类型的vector store，没有指定--LLM参数
LLM.CREATE-VECTOR-STORE key

// 创建一个vector store，并显式指定类型
LLM.CREATE-VECTOR-STORE key --TYPE hnsw

// 创建一个默认类型的vector store，设置一些参数
LLM.CREATE-VECTOR-STORE key --PARAMS '{"max_elements": 20000}'
```

### LLM.CREATE-APP

#### 语法

```
LLM.CREATE-APP key [--NX] [--XX] --LLM llm-key [--PROMPT prompt]
```

**LLM.CREATE-APP**创建一个简单应用（*simple application*），并保存到*key*处。你也可以为该应用指定一个prompt或者prompt模版。

#### 选项

- **--NX**: 当且仅当*key*不存在时，创建应用。可选。
- **--XX**: 当且仅当*key*存在时，创建应用。可选。
- **--LLM**: 应用使用的LLM模型在Redis中的key。
- **--PROMPT**: 应用的prompt或prompt模版，详情请查看[Prompt章节](#prompt)。

#### 返回值

- *Integer reply*: 如果应用创建成功，那么返回1，否则返回0（例如设置了*--NX*选项，但key已经存在了）。

#### 错误

以下情况会返回错误：

- 选项不正确。
- 存储在*key*处的数据不是应用。

#### 示例

```
// 创建一个不带prompt的应用
LLM.CREATE-APP key --LLM model-key

// 用给定的输入来运行
LLM.RUN key 'I want you to act as a poet. Please write a poet about love.'

// 使用硬编码的prompt来创建应用
LLM.CREATE-APP key --LLM model-key --PROMPT 'I want you to act as a poet. Please write a poet about love.'

// 运行该应用
LLM.RUN key

// 使用prompt模版来创建应用
LLM.CREATE-APP key --LLM model-key --PROMPT 'I want you to act as a poet. Please write a poet about {{domain}}.'

// 在运行时指定模版变量的值
LLM.RUN key --VARS '{"domain": "love"}'
```

### LLM.CREATE-SEARCH

#### 语法

```
LLM.CREATE-SEARCH key [--NX] [--XX] --LLM llm-key --VECTOR-STORE store-key [--K 3] [--PROMPT prompt]
```

**LLM.CREATE-SEARCH**创建一个*search application*，并保存在*key*处。该应用使用保存在*llm-key*处的LLM模型，对保存在*store-key*处的vector store中的数据进行检索。

#### 选项

- **--NX**: 当且仅当*key*不存在时，创建应用。可选。
- **--XX**: 当且仅当*key*不存在时，创建应用。可选。
- **--LLM**: 应用使用的LLM模型在Redis中的key。
- **--VECTOR-STORE**: 应用使用的vector store在Redis中的key。
- **--K**: 使用vector store中和输入最相近的K个数据作为检索的上下文。可选。如果没有指定，那么默认设置为3。K越大，通常检索的准确度越高，但是用的token数越多。
- **--PROMPT**: 该应用的Prompt模版

**注意**:

该prompt模版需要包含两个变量：*context*和*question*。当我们运行该应用时，会使用vector store中找到的相似数据做为*{{context}}*，用户的输入问题作为*{{question}}*。以下是默认模版：

```
Please answer the following question based on the given context.
Context: """
{{context}}
"""
Question: """
{{question}}
"""
Answer:
```

#### 返回值

- *Integer reply*: 如果应用创建成功，那么返回1，否则返回0（例如设置了*--NX*选项，但key已经存在了）。

#### 错误

以下情况会返回错误：

- 选项不正确。
- 存储在*key*处的数据不是应用。

#### 示例

```
// 创建一个search应用
LLM.CREATE-SEARCH key --LLM model-key --VECTOR-STORE store-key

// 给定输入，运行该应用
LLM.RUN key 'Question on your private data.'

// 创建一个search应用，指定更多的相似数据（--K 5）作为检索的上下文
LLM.CREATE-SEARCH key --LLM model-key --VECTOR-STORE store-key --K 5

// 运行该应用
LLM.RUN key 'Question on your private data.'
```

### LLM.CREATE-CHAT

#### 语法

```
LLM.CREATE-CHAT key [--NX] [--XX] --LLM llm-key --VECTOR-STORE store-key [--HISTORY {history conf in JSON format}] [--PROMPT prompt]
```

**LLM.CREATE-CHAT**创建一个*chat application*，并保存在*key*处。该应用使用保存在*llm-key*处的模型来和你对话，并把你们的聊天记录保存在*store-key*处的vector store中。

#### Options

- **--NX**: 当且仅当*key*不存在时，创建应用。可选。
- **--XX**: 当且仅当*key*存在时，创建应用。可选。
- **--LLM**: 应用使用的LLM模型在Redis中的key。
- **--VECTOR-STORE**: 应用使用的vector store在Redis中的key。
- **--HISTORY**: 聊天历史配置。可选。
- **--PROMPT**: 应用的prompt或prompt模版。可选。

**注意**:

该prompt模版包含了1个变量：*history*。当运行该应用时，*{{history}}*会被设置为和当前用户输入相关的聊天历史。以下是默认模版：

```
You are a friendly chatbot. The following is a summary of parts of your chat history with user: """
{{history}}
"""
```

以下是聊天历史的配置，同时给出了每个配置的默认值（这些默认值以后可能会修改）：

```
{"summary_cnt": 20, "summary_ctx_cnt": 1, "summary_prompt" : "Give a concise and comprehensive summary of the given conversation (in JSON format). The summary should capture the main points and supporting details.\nConversation: \"\"\"\n{{conversation}}\n\"\"\"\nSummary:", "msg_ctx_cnt": 10}
```

Chat application会对你最近的*summary_cnt*条消息进行总结，并将总结写入到vector store中。当你发送一条消息时，它会从vector store中检索*summary_ctx_cnt*条最相关的总结作为你和LLM模型聊天的历史记录。然后chat应用会将这个聊天历史（长期记忆）和最近的*msg_ctx_cnt*条消息（短期记忆）作为你们聊天上下文，结合当前的消息一起发送给LLM模型。通过这种方式LLM模型就能“记住”你们的聊天历史了。

如果*summary_cnt*设置为0，那么chat应用不会对你的聊天历史进行总结，也不会使用该总结作为聊天上下文。一次对话中，使用越多的历史总结，越多的最近消息记录，聊天的体验会越好（LLM可能知道越多的聊天上下文），但token开销也会越大。

#### 返回值

- *Integer reply*: 如果应用创建成功，那么返回1，否则返回0（例如设置了*--NX*选项，但key已经存在了）。

#### 错误

以下情况会返回错误：

- 选项不正确。
- 存储在*key*处的数据不是应用。

#### 示例

```
// 创建一个chat应用
LLM.CREATE-CHAT key --LLM model-key --VECTOR-STORE store-key

// 跟该应用对话
LLM.RUN key 'How are you?'

// 创建一个chat应用，并让应用对最近的20条记录进行总结
LLM.CREATE-SEARCH key --LLM model-key --VECTOR-STORE store-key --HISTORY '{"summary_cnt": 20}'
```

### LLM.ADD

#### 语法

```
LLM.ADD key [--ID id] [--EMBEDDING xxx] [--TIMEOUT in-milliseconds] data
```

**LLM.ADD**向存储在*key*处的vector store中写入一条数据。每条数据包含一个唯一的ID和embedding。

#### 选项

- **--ID**: 指定插入数据的ID（`uint64_t`类型）。如果ID已经存在了，那么覆盖它。可选。如果没有设置，redis-llm会自动生成一个ID。
- **--EMBEDDING**: 指定插入数据的embedding。可选。如果没有设置，那么redis-llm会调用LLM模型来生成一个embedding。
- **--TIMEOUT**: 客户端等待该操作的超时时间（毫秒）。可选。如果没有设置，那么默认是0，客户端会一直阻塞等待操作完成。

**注意**：如果超时时间到了，你无法知道该操作是否完成。

#### 返回值

- *Integer reply*: 插入数据的ID。
- *Nil reply*: 如果等待超时，返回nil。

#### 错误

以下情况会返回错误：

- *key*不存在。你需要在调用LLM.ADD之前调用LLM.CREATE-VECTOR-STORE
- 存储在*key*处的数据不是一个vector store
- vector store的大小达到了设置的*max_elements*限制
- vector store在创建的时候，没有指定*--LLM*选项，但执行LLM.ADD的时候没有指定*--EMBEDDING*。
- 使用LLM创建embedding失败。
- 显式指定的embedding的维度和vector store的维度不匹配。

#### 示例

以下使用创建了一个vector store，没有指定LLM，插入的时候，显式指定ID和embedding。

```
// 创建vector store，不指定LLM
LLM.CREATE-VECTOR-STORE store

// 插入一条记录，显式指定ID和embedding。由于是第一条插入的数据，因此，vecotr store
// 使用该数据的embedding的维度作为vector store的维度
LLM.ADD store --ID 1 --EMBEDDING 1,2,3 'some data'

// 以下插入会失败，因为维度不匹配
LLM.ADD store --ID 2 --EMBEDDING 1,2 'some other data'

// 以下插入会成功
LLM.ADD store --ID 2 --EMBEDDING 1,2,5 'some other data'
```

以下示例在创建vector store时指定了LLM，系统可以通过LLM来自动创建embedding。

```
// 使用保存在*model-key*处的LLM来创建一个vector store
LLM.CREATE-VECTOR-STORE store --LLM model-key

// 添加一条数据，使用LLM模型自动生成embedding，同时自动生成该数据的ID
LLM.ADD store 'some data'

// 添加一条数据，等待2秒钟，如果超时就提前返回
LLM.ADD store --TIMEOUT 2000 'some other data'
```

### LLM.GET

#### 语法

```
LLM.GET key id
```

**LLM.GET**从保存在*key*处的vector store中返回给定ID的数据和embedding。

#### 返回值

- **Array reply**: 2维数组。第一个元素是数据，第二个元素是embedding。
- **Nil reply**: 如果*key*或者*id*不存在，返回nil。

#### 错误

以下情况会返回错误：

- 存储在*key*处的数据不是vector store。

#### 示例

```
LLM.CREATE-VECTOR-STORE store

LLM.ADD store --ID 1 --EMBEDDING 1,2,3 'some data'

// 以下命令返回'some data'和1,2,3
LLM.GET store 1
```

### LLM.REM

#### 语法

```
LLM.REM key id
```

**LLM.REM**从vector store中删除给定ID的数据。

#### 返回值

- *Integer reply*: 如果数据存在并且删除成功了，那么返回1，否则返回0。

#### 错误

以下情况会返回错误：

- 存储在*key*处的数据不是vector store

#### 示例

```
LLM.CREATE-VECTOR-STORE store

LLM.ADD store --ID 1 --EMBEDDING 1,2,3 'some data'

LLM.REM store 1
```

### LLM.SIZE

#### 语法

```
LLM.SIZE key
```

**LLM.SIZE**返回存储在*key*处的vector store的大小。

#### 返回值

- *Integer reply*: vector store的大小。如果不存在，那么返回0。

#### 错误

以下情况会返回错误：

- 存储在*key*处的数据不是vector store

#### 示例

```
LLM.CREATE-VECTOR-STORE store

LLM.ADD store --ID 1 --EMBEDDING 1,2,3 'some data'

LLM.SIZE store
```

### LLM.KNN

#### 语法

```
LLM.KNN key [--K 10] [--EMBEDDING xxx] [--TIMEOUT timeout-in-milliseconds] [query]
```

**LLM.KNN**返回与给定embedding或者query最相似的K条数据。

#### 选项

**--K**: 返回的条数。可选。默认返回10条。
**--EMBEDDING**: 用于检索的embedding。可选。如果指定了，那么会从vector store中找出与该embedding最相似的K条记录。
- **--TIMEOUT**: 客户端等待该操作的超时时间（毫秒）。可选。如果没有设置，那么默认是0，客户端会一直阻塞等待操作完成。
**query**: 要查询的数据。可选。如果指定了，那么会利用LLM模型生成embedding，然后找出与其最相似的K条记录。

**注意**:
- 如果vector store的大小比K小，那么返回的条数小于K。
- *--EMBEDDING*和*query*都是可选的，但两者必须指定一个。

#### 返回值

- *Array reply*: 最多K条记录的ID和与输入embedding/query的距离。

#### 错误

以下情况会返回错误：

- *key*处保存的数据不是vector store
- *key*不存在

#### 示例

以下例子在插入时显式指定embedding，通过指定embedding来查询KNN。

```
LLM.CREATE-VECTOR-STORE store

LLM.ADD store --EMBEDDING 1,2,3 data1

LLM.ADD store --EMBEDDING 1,2,4 data2

LLM.ADD store --EMBEDDING 1,2,5 data3

LLM.KNN store --K 1 --EMBEDDING 1,2,3
```

以下例子让vector store自动生成embedding，通过给定query来查询KNN。

```
LLM.CREATE-VECTOR-STORE store --LLM model-key

LLM.ADD store data1

LLM.ADD store data2

LLM.ADD store data3

LLM.KNN store --K 2 data4
```

### LLM.RUN

#### 语法

```
LLM.RUN key [--VARS '{"variable" : "value"}'] [input]
```

**LLM.RUN**运行一个应用（simple application, search application or chat application)。

#### 选项

- **--VARS**: 如果应用指定了prompt模版，那么可以通过该选项来指定模版中变量的值。可选。

#### 返回值

- *Bulk string reply*: 应用的返回值。

#### 错误

以下情况会返回错误：

- 存储在*key*处的数据不是一个应用。
- 运行应用失败。

#### 示例

```
// 运行一个simple application
LLM.CREATE-APP translator --LLM llm-key --PROMPT 'Please translate the following text to Chinese: '

LLM.RUN translator 'What is LLM?'

// 运行一个search application.
LLM.CREATE-SEARCH searcher --LLM llm-key --VECTOR-STORE store-key

LLM.RUN searcher 'What is redis-plus-plus?'

// 运行一个chat application.
LLM.CREATE-CHAT chat --LLM llm-key --VECTOR-STORE store-key

LLM.RUN chat 'What is Redis?'
```

## 作者

redis-llm的作者是[sewenew](https://github.com/sewenew)，他也常常活跃在[StackOverflow](https://stackoverflow.com/users/5384363/for-stack)上。
