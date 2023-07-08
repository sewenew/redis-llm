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
    - [Vector Store](#vector-store)
    - [Application](#application)
- [命令](#命令)
    - [Path](#path)
- [作者](#作者)

## 概览

redis是一个[Redis模块](https://redis.io/topics/modules-intro)，它将大语言模型（LLM）集成到了Redis中。

大语言模型非常强大，但也有它自身的局限。

- 大语言模型无法访问用户的私有数据，不能基于这些私有数据来回答用户的问题。
- 大语言模型存在token数量的限制，不能记住很长的对话历史。

为了解决这些问题，我写了redis模块，将大语言模型集成到Redis中，使得我们可以把Redis作为大语言模型的外部存储。你可以将私有数据通过redis存储到Redis中，然后让大语言模型基于这些数据来回答问题。你也可以通过redis来和大语言模型对话，redis会记录并索引你们大对话历史，使得大语言模型可以记住几乎所有的对话内容。

### 功能

- 利用Prompt（提示词）来构建大语言模型应用
- 向量存储
- 针对私有数据进行提问
- 让大语言模型记住几乎所有的聊天记录

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

创建一个OpenAI类型的大语言模型。

```
127.0.0.1:6379> LLM.CREATE-LLM openai --params '{"api_key" : "$OPENAI_API_KEY"}'
(integer) 1
```

使用一个LLM和一段prompt创建一个*hello world*应用，并运行这个应用。

```
127.0.0.1:6379> LLM.CREATE-APP hello-world --LLM openai --PROMPT 'Say hello to {{name}}'
(integer) 1
127.0.0.1:6379> LLM.RUN hello-world --vars '{"name":"LLM"}'
"Hello LLM! It's nice to meet you. How can I assist you today?"
```

创建一个vector store，并往里面添加一段文档。

```
127.0.0.1:6379> LLM.CREATE-VECTOR-STORE store --LLM openai
(integer) 1
127.0.0.1:6379> LLM.ADD store 'redis-llm is a Redis module that integrates LLM (Large Language Model) with Redis'
(integer) 1
```

创建一个search（查询）应用，让它基于vector store中保存的数据来回答问题。

```
127.0.0.1:6379> LLM.CREATE-SEARCH search-private-data --LLM openai --VECTOR-STORE store
(integer) 1
127.0.0.1:6379> LLM.ADD store 'redis-llm is an open source project written by sewenew'
(integer) 1
127.0.0.1:6379> LLM.RUN search-private-data 'who is the author of redis-llm'
"The author of redis-llm is sewenew."
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

### LLM.CREATE-LLM

### LLM.ADD

#### 语法

```
LLM.ADD key [--ID id] [--EMBEDDING xxx] data
```

#### Options

#### Return Value

#### Error

#### Time Complexity

#### Examples

### LLM.GET

#### Syntax

```
LLM.GET key id
```

#### Options

#### Return Value

#### Error

#### Time Complexity

#### Examples

### LLM.REM

#### Syntax

```
LLM.REM key id
```

#### Options

#### Return Value

#### Error

#### Time Complexity

#### Examples

### LLM.SIZE

#### Syntax

```
LLM.SIZE key
```

#### Options

#### Return Value

#### Error

#### Time Complexity

#### Examples

### LLM.KNN

#### Syntax

```
LLM.KNN key [--K 10] [--embedding xxx] [query]
```

#### Options

#### Return Value

#### Error

#### Time Complexity

#### Examples

## 作者

redis-llm的作者是[sewenew](https://github.com/sewenew)，他也常常活跃在[StackOverflow](https://stackoverflow.com/users/5384363/for-stack)上。
