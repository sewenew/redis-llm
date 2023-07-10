# redis-llm

[中文文档](https://github.com/sewenew/redis-llm/blob/main/Chinese.md)

- [Overview](#overview)
    - [Features](#features)
- [Installation](#installation)
    - [Run redis-llm With Docker](#run-redis-llm-with-docker)
    - [Install redis-llm With Source Code](#install-redis-llm-with-source-code)
    - [Load redis-llm](#load-redis-llm)
- [Getting Started](#getting-started)
    - [redis-cli](#redis-cli)
    - [C++ Client](#c-client)
    - [Python Client](#python-client)
- [Terminology](#terminology)
    - [LLM](#llm)
    - [Prompt](#prompt)
    - [Vector Store](#vector-store-1)
    - [Application](#application)
- [Commands](#commands)
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
- [Author](#author)

## Overview

redis-llm is a [Redis Module](https://redis.io/topics/modules-intro) that integrates LLM (Large Language Model) with Redis.

LLM is powerful, but it’s still limited.

- It cannot access your private data, and cannot answer questions based on those data.
- It has token limitation, and cannot remember too much chat history.

In order to solve these problems, I write redis-llm to integrate LLM with Redis. This module makes Redis an extended memory of LLM. You can save your private data to Redis with redis-llm, and ask LLM to answer questions based on the data. You can also chat with redis-llm, which can save and index your chat history to help LLM chat with a long history.

### Features

- [Run LLM applications with prompt template](#simple-application)
- [Vector store](#vector-store)
- [Ask questions on private data saved in vector store](#search-application)
- [Chat with a long history](#chat-application).

## Installation

### Run redis-llm With Docker

Run the following command to start redis-llm with Docker.

```
docker run -p 6379:6379 sewenew/redis-llm:latest
```

In this case, Docker runs Redis with a *redis.conf* file located at */usr/lib/redis/conf/redis.conf*.

After running the Docker image, you can go to the [Getting Started section](#getting-started) to see how to run redis-llm commands.

### Install redis-protobuf With Source Code

You can also install redis-llm with source code.

redis-llm depends on curl and openssl, and you need to install these dependencies first.

```
apt-get install libssl-dev libcurl4-openssl-dev
```

redis-llm is built with [CMAKE](https://cmake.org).

```
git clone https://github.com/sewenew/redis-llm.git

cd redis-llm

mkdir compile

cd compile

cmake ..

make
```

When `make` is done, you should find *libredis-llm.so* (or *libredis-llm.dylib* on MacOS) under the *redis-llm/compile* directory.

### Load redis-llm

redis-llm module depends on Redis 5.0's module API, so you must install Redis 5.0 or above.

In order to load redis-llm, you need to modify the *redis.conf* file to add the `loadmodule` directive:

```
loadmodule /path/to/libredis-llm.so
```

Now, you can start your Redis instance:

```
redis-server /path/to/redis.conf
```

If Redis loads the module successfully, you can get the following message from the log:

```
Module 'LLM' loaded from /path/to/libredis-llm.so
```

#### Module Options

redis-llm uses a thread pool to do time-consuming jobs. These jobs are submitted to a task queue, and threads in the pool fetch tasks to run. You can set the queue size (number of tasks) and pool size (number of threads) with the following options when loading redis-llm module:

- **--QUEUE_SIZE**: Size of the task queue. Optional. The default size is 1000.
- **--POOL_SIZE**: Size of the thread pool. Optional. The default size is 10.

```
loadmodule /path/to/libredis-llm.so --QUEUE_SIZE 3000 --POOL_SIZE 20
```

## Getting Started

After [loading the module](#load-redis-llm), you can use any Redis client to send redis-llm [commands](#Commands).

### redis-cli

The following examples use the offical Redis client, i.e. redis-cli, to send redis-llm commands.

**NOTE**: If you send command with non-English, you should launch redis-cli with *--raw* option: `redis-cli --raw`.

List module info:

```
127.0.0.1:6379> MODULE LIST
1) 1) "name"
   2) "LLM"
   3) "ver"
   4) (integer) 1
```

Create an LLM model of OpenAI type by specifying your openai API key.

```
127.0.0.1:6379> LLM.CREATE-LLM model --TYPE openai --PARAMS '{"api_key" : "$OPENAI_API_KEY"}'
(integer) 1
```

#### Vector Store

Create a vector store without LLM support and you need to add data to store with embedding.

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

Create a vector store with LLM support, and the store automatically generate embedding with LLM.

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

Create a *hello world* application with LLM, and run it with input.

```
127.0.0.1:6379> LLM.CREATE-LLM model --TYPE openai --PARAMS '{"api_key" : "$OPENAI_API_KEY"}'
(integer) 1
127.0.0.1:6379> LLM.CREATE-APP hello-world --LLM model
(integer) 1
127.0.0.1:6379> LLM.RUN hello-world 'Say hello to LLM'
"Hello LLM! It's nice to meet you. How can I assist you today?"
```

Create a *hello world* application with LLM and prompt (You must call LLM.CREATE-LLM to create LLM model beforehand).

```
127.0.0.1:6379> LLM.CREATE-APP hello-world --LLM model --PROMPT 'Say hello to LLM'
(integer) 1
127.0.0.1:6379> LLM.RUN hello-world
"Hello LLM! It's nice to meet you. How can I assist you today?"
```

Create a *hello world* application with LLM and prompt template (You must call LLM.CREATE-LLM to create LLM model beforehand). Then you can run it by setting template variables.

```
127.0.0.1:6379> LLM.CREATE-APP hello-world --LLM model --PROMPT 'Say hello to {{name}}'
(integer) 1
127.0.0.1:6379> LLM.RUN hello-world --vars '{"name":"LLM"}'
"Hello LLM! It's nice to meet you. How can I assist you today?"
```

#### Search Application

Create a search application which can answer questions based on your private data stored in the vector store.

- Call LLM.CREATE-LLM to create an LLM model.
- Call LLM.CREATE-VECTOR-STORE to create a vector store with LLM support.
- Call LLM.ADD to add your private data to the store.
- Call LLM.CREATE-SEARCH to create a search application with LLM and vector store.
- Call LLM.RUN to ask questions on your private data.

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

Check [this](https://github.com/sewenew/redis-llm/tree/main/examples/search-application) for examples on building a private data searcher on Redis commands and redis-plus-plus.

#### Chat Application

Create a chat application which help LLM remember a long conversation history.

- Call LLM.CREATE-LLM to create an LLM model.
- Call LLM.CREATE-VECTOR-STORE to create a vector store with LLM support. Chat application stores conversation history in this vector store.
- Call LLM.CREATE-CHAT to create a chat application with LLM and vector store.
- Call LLM.RUN to chat with LLM.

```
127.0.0.1:6379> LLM.CREATE-LLM model --TYPE openai --PARAMS '{"api_key" : "$OPENAI_API_KEY"}'
(integer) 1
127.0.0.1:6379> LLM.CREATE-VECTOR-STORE history --LLM model
(integer) 1
127.0.0.1:6379> LLM.CREATE-CHAT chat --LLM model --VECTOR-STORE history
(integer) 1
127.0.0.1:6379> LLM.RUN chat 'Can you recommend a C++ Redis client library for me?'
```

### C++ Client

If you are using C++, you can use [redis-plus-plus](https://github.com/sewenew/redis-plus-plus) to send redis-llm commands:

```C++
```

### Python Client

If you are using Python, you can use [redis-py](https://github.com/redis/redis-py) to send redis-llm commands:

```Python
```

## Terminology

You can use redis-llm to create applications with LLM, e.g. chat bot. The following are some terminologies.

### LLM

Large Language Model.

Currently, we support the following LLMs:

- [OpenAI](https://platform.openai.com/docs/api-reference)
- [Azure OpenAI](https://azure.microsoft.com/en-us/products/cognitive-services/openai-service)

If you need other LLMs support, feel free to leave a message [here](https://github.com/sewenew/redis-llm/issues/2).

### Prompt

You input some *prompt*, and LLMs return a text *completion*. A good prompt makes a good completion. Check [this](https://help.openai.com/en/articles/6654000-best-practices-for-prompt-engineering-with-openai-api) for some best practices for prompt engineering.

The following command creates an LLM application with a prompt:

```
LLM.CREATE-APP test-application --PROMPT 'You are an expert on prompt engineering. Please give some best practices for prompt engineering.'
```

Once you run the application, it gives you text completion, i.e. best practices for prompt engineering, based on your prompt.

```
LLM.RUN test-application
```

#### Prompt Template

Besides hard-coded prompt, redis-llm also supports prompt template. You can create a prompt template with some variables. With different values for these variables, you can get a dynamic template.

The following command creates an LLM application with a prompt template (NOTE: in order to make the following example work, you need to create an LLM named *llm* beforehand):

```
LLM.CREATE-APP app-with-prompt-template --LLM llm-key --PROMPT 'You are an expert on {{domain}}. Please answer the following question: {{question}}'
```

The substring between *{{* and *}}* is a template variable. In the above example, there're two variables, *domain* and *question*. When you run the application, you can set different values for these variables to make a dynamic template.

```
LLM.RUN app-with-prompt-template --LLM llm-key --VARS '{"domain" : "LLM", "question" : "Please give an introduction on LLM."}'
```

With *--VARS* option, we set values for each variable. The option is in JSON format, and each key-value pair in JSON string corresponds to a variable. redis-llm renders the above template into the following prompt, and ask LLM for completion.

```
You are an expert on LLM. Please answer the following question: Please give an introduction on LLM.
```

### Vector Store

Vector store is a database that store data with their embeddings, i.e. vector. Embedding can capture semantics of the data, and data with similar embeddings are semantically similar. Normally, we use Approximate Nearest Neighbor (ANN) algorithms to search the K approximatly nearest items of a given input.

There are many ANN algorithms, and currently, we support the following ones:

- [HNSW](https://github.com/nmslib/hnswlib)

### Application

With redis-llm, you can create LLM applications.

Currently, we support the following applications:

- **Simple Application**: Application with a prompt or prompt template. You can run the application with an input string (prompt), and it returns an output string (completion). Check [LLM.CREATE-APP command](#llmcreate-app) for detail.
- **Search Application**: Application which can answer questions on a given vector store (you can store your private data in the vector store). Check [LLM.CREATE-SEARCH command](#llmcreate-search) for detail.
- **Chat Application**: Application which helps you chat with LLM. Chat Application indexes the chat history, and helps LLM remember your conversation history. Check [LLM.CREATE-CHAT command](#llmcreate-chat) for detail.

## Commands

Command names and option names are case insensitive.

### LLM.CREATE-LLM

#### Syntax

```
LLM.CREATE-LLM key [--NX] [--XX] [--TYPE model-type] --PARAMS '{model parameters in JSON format}'
```

**LLM.CREATE-LLM** creates an LLM model stored at *key*. Since all applications use LLM model to do query or embedding, you must create an LLM model before creating application.

#### Options

- **--NX**: Create model, if and only if *key* does not exist. Optional.
- **--XX**: Create model, if and only if *key* exists. Optional.
- **--TYPE**: Model type. Optional. If not specified, the default type is *openai*.
- **--PARAMS**: Model parameters in JSON format. Check [LLM Models section](#llm-models) for detail.

#### LLM Models

##### openai

If you want to use OpenAI, you should specify `--TYPE openai`. The parameters are as follows:

```JSON
{"api_key": "required", "chat_path": "/v1/chat/completions", "chat": {"model": "gpt-3.5-turbo"}, "embedding_path": "/v1/embeddings", "embedding": {"model":"text-embedding-ada-002"}, "http":{"socket_timeout":"5s","connect_timeout":"5s", "enable_certificate_verification":false, "proxy_host": "", "proxy_port": 0, "pool" : {"size": 5, "wait_timeout":"0s", "connection_lifetime":"0s"}}}
```

All parameters are key-value pairs. The required ones are set as *required*. The optional ones are set with default values. If parameter is not specified, the default value is used. For example, if you want to use *gpt-3.5-turbo-0301* model, and use default values for other optional parameters:

```
LLM.CREATE-LLM key --PARAMS '{"api_key" : "sk-your-api-key", "chat": {"model": "gpt-3.5-turbo-0301"}}'
```

If you want to set [other parameters](https://platform.openai.com/docs/api-reference/chat/create) for chat or embedding API, simply put them into the *chat* part. The following example sets *api_key* and the *temperature* parameter for chat API:

```
LLM.CREATE-LLM key --PARAMS '{"api_key" : "sk-your-api-key", "chat": {"temperature": 0.5}}'
```

##### azure openai

If you want to use Azure OpenAI, you should specify `--TYPE azure_openai`. The parameters are as follows:

```JSON
{"api_key": "required", "resource_name" : "required", "chat_deployment_id": "required", "embedding_deployment_id": "required", "api_version": "required", "chat": {}, "embedding": {}, "http": {"socket_timeout":"5s", "connect_timeout":"5s", "enable_certificate_verification": false, "proxy_host": "", "proxy_port": 0, "pool" : {"size": 5, "wait_timeout":"0s", "connection_lifetime":"0s"}}}
```

All parameters are key-value pairs. The required ones are set as *required*. The optional ones are set with default values. If parameter is not specified, the default value is used. For example, if you want to use set *socket_time* to 10s, and use default values for other optional parameters:

```
LLM.CREATE-LLM key --TYPE azure_openai --PARAMS '{"api_key" : "sk-your-api-key", "resource_name": "your-resource_name", "chat_deployment_id": "your-chat_deployment_id", "embedding_deployment_id": "your-embedding_deployment_id", "api_version" : "api-version", "http": {"socket_timeout" : "10s"}}'
```

If you want to set [other parameters](https://learn.microsoft.com/en-us/azure/cognitive-services/openai/reference) for chat or embedding API, simply put them into the *chat* part. The following example sets *temperature* parameter for chat API:

```
LLM.CREATE-LLM key --TYPE azure_openai --PARAMS '{"api_key" : "sk-your-api-key", "resource_name": "your-resource_name", "chat_deployment_id": "your-chat_deployment_id", "embedding_deployment_id": "your-embedding_deployment_id", "api_version" : "api-version", "chat": {"temperature" : 0.5}}'
```

#### Return

- *Integer reply*: 1 if creating model OK. 0, otherwise, e.g. option *--NX* has been set, while the key already exists.

#### Error

Return an error reply in the following cases:

- Model type does not supported.
- Cannot create a model with the given parameters.
- Data stored at *key* is not a LLM model.

#### Examples

```
// Create a LLM model of openai type. Set its api key, and use default values for other parameters
LLM.CREATE-LLM key --TYPE openai --PARAMS '{"api_key": "Your API KEY"}'

// Create a LLM model of openai type. Set api key, and chat model.
LLM.CREATE-LLM key --TYPE openai --PARAMS '{"api_key": "Your API KEY", "chat" : {"model" : "gpt-3.5-turbo-0301"}}'

// Create a LLM model of azure openai type. Set required parameters.
LLM.CREATE-LLM key --TYPE azure_openai --PARAMS '{"api_key": "Your API KEY", "resource_name": "your resource name", "chat_deployment_id": "your deployment id for chat api", "embedding_deployment_id": "your deployment id for embedding api", "api_version": "api version"}'

// Create a LLM model of azure openai type. Set required parameters, and set http proxy to access azure openai.
LLM.CREATE-LLM key --TYPE azure_openai --PARAMS '{"api_key": "Your API KEY", "resource_name": "your resource name", "chat_deployment_id": "your deployment id for chat api", "embedding_deployment_id": "your deployment id for embedding api", "api_version": "api version", "http": {"proxy_host": "http://xxx.xxx.xx", "proxy_port": 3149}}'
```

### LLM.CREATE-VECTOR-STORE

#### Syntax

```
LLM.CREATE-VECTOR-STORE key [--NX] [--XX] [--TYPE vector-store-type] [--LLM llm-info] [--PARAMS '{store parameters in JSON format}']
```

**LLM.CREATE-VECTOR-STORE** creates a vector store stored at *key*. If you want to LLM application to access your private data, you can save the data into a vector store.

#### Options

- **--NX**: Create store, if and only if *key* does not exist. Optional.
- **--XX**: Create store, if and only if *key* exists. Optional.
- **--TYPE**: Vector store type. Optional. If not specified, the default type is *hnsw*. Optional.
- **--LLM**: Redis key of LLM model that you want to use with this vector store. When you use LLM.ADD command without a given embedding, redis-llm uses this LLM model to build embedding, and add it into the store.
- **--PARAMS**: Vector store parameters in JSON format. Check [Vector Stores section](#vector-stores) for detail. Optional.

#### Vector Stores

##### hnsw

Currently, we only support vector store of HNSW type, and this is also the default one. Of course, you can specify `--TYPE hnsw` explicitly. The parameters are as follows:

```JSON
{"max_elements": 100000, "m": 16, "ef_construction": 200, "dim": 0}
```

All parameters are key-value pairs. The required ones are set as *required*. The optional ones are set with default values. If parameter is not specified, the default value is used.

- *max_elements*: Max number of items that can be stored in the vector store.
- *dim*: Set the dimension of the vector saved in vector store. If it's 0, i.e. the default value, the dimension of the first inserted vector is used as the dimension of the vector store.

#### Return

- *Integer reply*: 1 if creating vector store OK. 0, otherwise, e.g. option *--NX* has been set, while the key already exists.

#### Error

Return an error reply in the following cases:

- Vector store type does not supported.
- Cannot create a vector store with the given parameters.
- Data stored at *key* is not a vector store.

#### Examples

The following examples create a vector store with LLM support, i.e. --LLM option is specified. And you can store data with or without embedding created beforehand. If no embedding is given, vector store use the LLM to create embedding automatically. Check [LLM.ADD command](#llmadd-command) for detail.

```
// Create a vector store of default type, i.e. hnsw.
LLM.CREATE-VECTOR-STORE key --LLM llm-key

// Create a vector store by explicitly set hnsw as type.
LLM.CREATE-VECTOR-STORE key --TYPE hnsw --LLM llm-key

// Create a vector store, and set some parameters.
LLM.CREATE-VECTOR-STORE key --LLM llm-key --PARAMS '{"max_elements": 20000}'
```

In some cases, you've already have embeddings for your data, and do not need vector store to call LLMs. In those cases, you don't need to set the *--LLM* parameter. The following are some examples creating a vector store without LLM support, i.e. no --LLM option is specified.

```
// Create a vector store of default type, i.e. hnsw.
LLM.CREATE-VECTOR-STORE key

// Create a vector store by explicitly set hnsw as type.
LLM.CREATE-VECTOR-STORE key --TYPE hnsw

// Create a vector store, and set some parameters.
LLM.CREATE-VECTOR-STORE key --PARAMS '{"max_elements": 20000}'
```

### LLM.CREATE-APP

#### Syntax

```
LLM.CREATE-APP key [--NX] [--XX] --LLM llm-info [--PROMPT prompt]
```

**LLM.CREATE-APP** creates a *simple application* stored at *key*. You can also set a prompt or prompt template for the application.

#### Options

- **--NX**: Create applicaiton, if and only if *key* does not exist. Optional.
- **--XX**: Create application, if and only if *key* exists. Optional.
- **--LLM**: Redis key of LLM model that this application uses. Required.
- **--PROMPT**: Prompt or prompt template for this application. Check [Prompt section](#prompt) for detail. Optional.

#### Return

- *Integer reply*: 1 if creating simple application OK. 0, otherwise, e.g. option *--NX* has been set, while the key already exists.

#### Error

Return an error reply in the following cases:

- Invalid options.
- Data stored at *key* is not an application.

#### Examples

```
// Create an application without prompt.
LLM.CREATE-APP key --LLM model-key

// Run it with input.
LLM.RUN key 'I want you to act as a poet. Please write a poet about love.'

// Create an application with hard-coded prompt.
LLM.CREATE-APP key --LLM model-key --PROMPT 'I want you to act as a poet. Please write a poet about love.'

// Run it.
LLM.RUN key

// Create an application with prompt template.
LLM.CREATE-APP key --LLM model-key --PROMPT 'I want you to act as a poet. Please write a poet about {{domain}}.'

// Run it with variables.
LLM.RUN key --VARS '{"domain": "love"}'
```

### LLM.CREATE-SEARCH

#### Syntax

```
LLM.CREATE-SEARCH key [--NX] [--XX] --LLM llm-key --VECTOR-STORE store-key [--K 3] [--PROMPT prompt]
```

**LLM.CREATE-SEARCH** creates a *search application* stored at *key*. The application uses LLM model stored at *llm-key* to search to your private data stored at *store-key*.

#### Options

- **--NX**: Create application, if and only if *key* does not exist. Optional.
- **--XX**: Create application, if and only if *key* exists. Optional.
- **--LLM**: Redis key of LLM model that this application uses. Required.
- **--VECTOR-STORE**: Redis key of vector store that this application uses. Required.
- **--K**: Number of similiar items in the vector store used as context for searching. Optional. If not specified, use 3 items as context. Larger K, might get a better answer, while costs more tokens.
- **--PROMPT**: Prompt template for this application.

**NOTE**:

The prompt template should contain 2 variables: *context* and *question*. When running the application, *{{context}}* will be set with similar items from the vector store, and the *{{question}}* will be set with the input question. The following is the default prompt template:

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

#### Return

- *Integer reply*: 1 if creating search application OK. 0, otherwise, e.g. option *--NX* has been set, while the key already exists.

#### Error

Return an error reply in the following cases:

- Invalid options.
- Data stored at *key* is not an application.

#### Examples

```
// Create a search application with LLM model and vector store.
LLM.CREATE-SEARCH key --LLM model-key --VECTOR-STORE store-key

// Run it with input.
LLM.RUN key 'Question on your private data.'

// Create a search application with more context info, i.e. --K 5.
LLM.CREATE-SEARCH key --LLM model-key --VECTOR-STORE store-key --K 5

// Run it.
LLM.RUN key 'Question on your private data.'
```

### LLM.CREATE-CHAT

#### Syntax

```
LLM.CREATE-CHAT key [--NX] [--XX] --LLM llm-key --VECTOR-STORE store-key [--HISTORY {history conf in JSON format}] [--PROMPT prompt]
```

**LLM.CREATE-CHAT** creates a *chat application* stored at *key*. The application uses LLM model stored at *llm-key* to chat with you, and index your conversation history in a vector store stored at *store-key*.

#### Options

- **--NX**: Create application, if and only if *key* does not exist. Optional.
- **--XX**: Create application, if and only if *key* exists. Optional.
- **--LLM**: Redis key of LLM model that this application uses. Required.
- **--VECTOR-STORE**: Redis key of vector store that this application uses. Required.
- **--HISTORY**: Configuration on how to index conversation history. Optional.
- **--PROMPT**: Prompt template for this application. Optional.

**NOTE**:

The prompt template should contain 1 variables: *history*. When running the application, *{{history}}* will be set with conversation history summaries related to user input. The following is the default prompt template:

```
You are a friendly chatbot. The following is a summary of parts of your chat history with user: """
{{history}}
"""
```

The following is the history configuration and related default values (these default values might be changed in the future):

```
{"summary_cnt": 20, "summary_ctx_cnt": 1, "summary_prompt" : "Give a concise and comprehensive summary of the given conversation (in JSON format). The summary should capture the main points and supporting details.\nConversation: \"\"\"\n{{conversation}}\n\"\"\"\nSummary:", "msg_ctx_cnt": 10}
```

Chat application summarize your latest *summary_cnt* messages, and store it into the vector store. When you send a message, it searches *summary_ctx_cnt* nearest summaries from the vector store as your conversation history. Finally, it uses both the conversation history (long term) and latest *msg_ctx_cnt* messages (short term) as context, and send your input message to LLM for completion. In this way, LLM can "remember" your conversation history.

If *summary_cnt* is 0, chat application does not summarize your conversation, and does not use long term history as context. The more history summaries, the more latest messages, the better conversation experience (LLM knows more conversation context), but the more cost.

#### Return

- *Integer reply*: 1 if creating search application OK. 0, otherwise, e.g. option *--NX* has been set, while the key already exists.

#### Error

Return an error reply in the following cases:

- Invalid options.
- Data stored at *key* is not an application.

#### Examples

```
// Create a chat application with LLM model and vector store.
LLM.CREATE-CHAT key --LLM model-key --VECTOR-STORE store-key

// Chat with the application.
LLM.RUN key 'How are you?'

// Create a chat application with converation summary enabled.
LLM.CREATE-SEARCH key --LLM model-key --VECTOR-STORE store-key --HISTORY '{"summary_cnt": 20}'
```

### LLM.ADD

#### Syntax

```
LLM.ADD key [--ID id] [--EMBEDDING xxx] [--TIMEOUT in-milliseconds] data
```

**LLM.ADD** adds *data* into the vector store stored at *key*. Each item in the vector store has a unique ID, and an embedding.

#### Options

- **--ID**: Specify an ID of `uint64_t` type for the data. If ID already exists, overwrite it. Optional. If not specified, redis-llm automatically generates an ID for the given data.
- **--EMBEDDING**: Specify embedding for the data. Optional. If not specified, redis-llm calls LLM of the vector store to create an embedding.
- **--TIMEOUT**: Operation timeout in milliseconds. Optional. If not specified, i.e. 0ms, client blocks until the operation finishes.

**NOTE**: If timeout reaches, you cannot tell whether the item has been added or not.

#### Return

- *Integer reply*: ID of the inserted data.
- *Nil reply*: If the operation is timed out.

#### Error

Return an error reply in the following cases:

- *key* does not exist. You should call LLM.CREATE-VECTOR-STORE beforehand.
- Data stored at *key* is NOT a vector store.
- Size of the vector store reaches *max_elements* limit.
- The vector store is not created with *--LLM*, while LLM.ADD runs without *--EMBEDDING*.
- Failed to create embedding with LLM.
- Explicitly added embedding's dimension does not match the embedding dimension of the vector store.

#### Examples

The following examples create a vector store, and add data with explicit ID and embedding.

```
// Create a vector store without LLM.
LLM.CREATE-VECTOR-STORE store

// Add a item to store, with ID, embedding and data. Sinc this is the first item
// added to the store, the dimension of the store is set to be the dimension of
// the embedding, i.e. 3.
LLM.ADD store --ID 1 --EMBEDDING 1,2,3 'some data'

// The following command will fail, since the added embedding does not match the dimension of the store.
LLM.ADD store --ID 2 --EMBEDDING 1,2 'some other data'

// The following command will success.
LLM.ADD store --ID 2 --EMBEDDING 1,2,5 'some other data'
```

The following examples create a vector store, and add data with automatically generated ID and embedding.

```
// Create a vector store with an LLM model stored at *model-key*.
LLM.CREATE-VECTOR-STORE store --LLM model-key

// Add data to the store. redis-llm uses LLM model to create an embedding, and adds
// it into the store with an automatically generated ID.
LLM.ADD store 'some data'

// Add data to the store, and set 2 seconds timeout for the operation.
LLM.ADD store --TIMEOUT 2000 'some other data'
```

### LLM.GET

#### Syntax

```
LLM.GET key id
```

**LLM.GET** return the data and embedding from vector store stored at *key* with the given *id*.

#### Return

- **Array reply**: 2-dimension array. The first item is the data, and the second one is embedding.
- **Nil reply**: If *key* or *id* does not exist.

#### Error

Return an error reply in the following cases:

- Data stored at *key* is a vector store.

#### Examples

```
LLM.CREATE-VECTOR-STORE store

LLM.ADD store --ID 1 --EMBEDDING 1,2,3 'some data'

// The following command should return 'some data' and 1,2,3
LLM.GET store 1
```

### LLM.REM

#### Syntax

```
LLM.REM key id
```

**LLM.REM** removes an item from vector store stored at *key* with the given *id*.

#### Return

- *Integer reply*: 1 if the item exist, and removed OK. 0, otherwise.

#### Error

Return an error reply in the following cases:

- Data stored at *key* is a vector store.

#### Examples

```
LLM.CREATE-VECTOR-STORE store

LLM.ADD store --ID 1 --EMBEDDING 1,2,3 'some data'

LLM.REM store 1
```

### LLM.SIZE

#### Syntax

```
LLM.SIZE key
```

**LLM.SIZE** returns the size of the vector store stored at *key*.

#### Return

- *Integer reply*: The size of the store. 0, if the *key* does not exist.

#### Error

Return an error reply in the following cases:

- Data stored at *key* is a vector store.

#### Examples

```
LLM.CREATE-VECTOR-STORE store

LLM.ADD store --ID 1 --EMBEDDING 1,2,3 'some data'

LLM.SIZE store
```

### LLM.KNN

#### Syntax

```
LLM.KNN key [--K 10] [--EMBEDDING xxx] [--TIMEOUT timeout-in-milliseconds] [query]
```

**LLM.KNN** returns K approximatly nearest items in vector store with the given embedding or query.

#### Options

**--K**: Number of items to be returned. Optional. If not specified, return 10 items.
**--EMBEDDING**: Embedding to be searched. Optional. If specified, redis-llm finds the K approximatly nearest items of the embedding.
- **--TIMEOUT**: Operation timeout in milliseconds. 0, by default. Optional. If not specified, i.e. 0ms, client blocks until the operation finishes.
**query**: Query data to be searched. Optional. If specified, redis-llm uses LLM to create embedding of the query, and finds the K approximatly nearest items.

**NOTE**:
- The number of returned items is less than *k*, if the size of the vector store is smaller than *K*.
- Both *--EMBEDDING* and *query* are optional, but you must specify one of them.

#### Return

- *Array reply*: At most K nearest items' ID and distance from the given embedding or query.

#### Error

Return an error reply in the following cases:

- Data stored at *key* is not a vector store.
- Vector store does not exist.

#### Examples

The following examples explicitly store embeddings with data, and retrieve KNN with an ebedding.

```
LLM.CREATE-VECTOR-STORE store

LLM.ADD store --EMBEDDING 1,2,3 data1

LLM.ADD store --EMBEDDING 1,2,4 data2

LLM.ADD store --EMBEDDING 1,2,5 data3

LLM.KNN store --K 1 --EMBEDDING 1,2,3
```

The following examples store data with automatically generated embeddings, and retrieve KNN with data query.

```
LLM.CREATE-VECTOR-STORE store --LLM model-key

LLM.ADD store data1

LLM.ADD store data2

LLM.ADD store data3

LLM.KNN store --K 2 data4
```

### LLM.RUN

#### Syntax

```
LLM.RUN key [--VARS '{"variable" : "value"}'] [input]
```

**LLM.RUN** runs an application, e.g. simple application, search application or chat application.

#### Options

- **--VARS**: If the application has a prompt template, you can use this option to set variables. Optional.

#### Return

- *Bulk string reply*: Result of the application.

#### Error

Return an error reply in the following cases:

- Data stored at *key* is not an application.
- Failed to run the application.

#### Examples

```
// Run a simple application.
LLM.CREATE-APP translator --LLM llm-key --PROMPT 'Please translate the following text to Chinese: '

LLM.RUN translator 'What is LLM?'

// Run a search application.
LLM.CREATE-SEARCH searcher --LLM llm-key --VECTOR-STORE store-key

LLM.RUN searcher 'What is redis-plus-plus?'

// Run a chat application.
LLM.CREATE-CHAT chat --LLM llm-key --VECTOR-STORE store-key

LLM.RUN chat 'What is Redis?'
```

## Author

redis-llm is written by [sewenew](https://github.com/sewenew), who is also active on [StackOverflow](https://stackoverflow.com/users/5384363/for-stack).
