# create search application

You can use redis-llm to create a search application which can answer questions based on your private data stored in the vector store. Normally, you need to run the following steps:

- Call LLM.CREATE-LLM to create an LLM model.
- Call LLM.CREATE-VECTOR-STORE to create a vector store with LLM support.
- Call LLM.ADD to add your private data to the store.
- Call LLM.CREATE-SEARCH to create a search application with LLM and vector store.
- Call LLM.RUN to ask questions on your private data.

I splitted [Redis' commands documentation](https://github.com/redis/redis-doc/tree/master/commands), and [redis-plus-plus (C++ Redis client)'s README.md](https://github.com/sewenew/redis-plus-plus/blob/master/README.md) into segmentations. For each segmentation, I saved it into a file, and called OpenAI embedding API (model: text-embedding-ada-002) to generate their embeddings. These data and embeddings are located at *examples/search-application/embeddings.zip*. You can unzip it to add into vector store.

```
unzip embeddings.zip
```

```
ls embeddings
data				redis-cmds-embedding		redis-plus-plus-doc-embeddings
```

You can find all segmentation files under *data* directory. *redis-cmds-embedding* and *redis-plus-plus-doc-embeddings* are embeddings files. Each embedding file contains pre-generated embeddings. Each line of the file contains data file name and data embedding, separated with "\t".

You can use *examples/search-application/create-search-app.sh* script to run create LLM model and vector store, add pre-genereated embeddings to the store, and create a search application. The following is the usage:

```Shell
create-search-app.sh embedding-file data-path store-key llm-key openai-api-key search-key
```

- *embedding-file*: Path to embedding file. e.g. examples/search-application/embeddings/redis-plus-plus-doc-embeddings
- *data-path*: Path to data directory, e.g. examples/search-application/embeddings/data.
- *store-key*: Redis key for the created vector store.
- *llm-key*: Redis key for the created LLM model.
- *openai-api-key*: Your OpenAI API key.
- *search-key*: Redis key for the created search application.

Ensure you've already installed `redis-cli`, and launched Redis server on localhost with redis-llm module loaded:

```
redis-cli module list
1) 1) "name"
   2) "LLM"
   3) "ver"
   4) (integer) 1
```

## Create a search application to ask question on Redis commands' documentation

Run the following command to load Redis documenation and embeddings:

```
cd /path/to/redis-llm/examples/search-application

sh create-search-app.sh embeddings/redis-cmds-embedding embeddings/data redis-cmd-store openai-model $YOUR-OPENAI-API-KEY redis-cmd-searcher
```

Ask questions on how to use Redis commands:

```
redis-cli llm.run redis-cmd-searcher 'How to run set command?'
```

## Create a search application to ask question on redis-plus-plus' documentation

Run the following command to load redis-plus-plus' documenation and embeddings:

```
cd /path/to/redis-llm/examples/search-application

sh create-search-app.sh embeddings/redis-plus-plus-doc-embeddings embeddings/data redis-plus-plus-doc-store openai-model $YOUR-OPENAI-API-KEY redis-plus-plus-searcher
```

Ask questions on redis-plus-plus:

```
redis-cli llm.run redis-plus-plus-searcher 'Who is the author of redis-plus-plus?'
```
