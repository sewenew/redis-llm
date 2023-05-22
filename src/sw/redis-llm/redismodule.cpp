// This file is copied from Redis 4.0, and I splited the original header into
// a header file and a cpp file, i.e. redismodule.h and redismodule.cpp.
// So that it can be compiled with C++ compiler.

#include "redismodule.h"

#ifndef REDISMODULE_CORE

void *REDISMODULE_API_FUNC(RedisModule_Alloc)(size_t bytes);
void *REDISMODULE_API_FUNC(RedisModule_Realloc)(void *ptr, size_t bytes);
void REDISMODULE_API_FUNC(RedisModule_Free)(void *ptr);
void *REDISMODULE_API_FUNC(RedisModule_Calloc)(size_t nmemb, size_t size);
char *REDISMODULE_API_FUNC(RedisModule_Strdup)(const char *str);
int REDISMODULE_API_FUNC(RedisModule_GetApi)(const char *, void *);
int REDISMODULE_API_FUNC(RedisModule_CreateCommand)(RedisModuleCtx *ctx, const char *name, RedisModuleCmdFunc cmdfunc, const char *strflags, int firstkey, int lastkey, int keystep);
void REDISMODULE_API_FUNC(RedisModule_SetModuleAttribs)(RedisModuleCtx *ctx, const char *name, int ver, int apiver);
int REDISMODULE_API_FUNC(RedisModule_IsModuleNameBusy)(const char *name);
int REDISMODULE_API_FUNC(RedisModule_WrongArity)(RedisModuleCtx *ctx);
int REDISMODULE_API_FUNC(RedisModule_ReplyWithLongLong)(RedisModuleCtx *ctx, long long ll);
int REDISMODULE_API_FUNC(RedisModule_GetSelectedDb)(RedisModuleCtx *ctx);
int REDISMODULE_API_FUNC(RedisModule_SelectDb)(RedisModuleCtx *ctx, int newid);
void *REDISMODULE_API_FUNC(RedisModule_OpenKey)(RedisModuleCtx *ctx, RedisModuleString *keyname, int mode);
void REDISMODULE_API_FUNC(RedisModule_CloseKey)(RedisModuleKey *kp);
int REDISMODULE_API_FUNC(RedisModule_KeyType)(RedisModuleKey *kp);
size_t REDISMODULE_API_FUNC(RedisModule_ValueLength)(RedisModuleKey *kp);
int REDISMODULE_API_FUNC(RedisModule_ListPush)(RedisModuleKey *kp, int where, RedisModuleString *ele);
RedisModuleString *REDISMODULE_API_FUNC(RedisModule_ListPop)(RedisModuleKey *key, int where);
RedisModuleCallReply *REDISMODULE_API_FUNC(RedisModule_Call)(RedisModuleCtx *ctx, const char *cmdname, const char *fmt, ...);
const char *REDISMODULE_API_FUNC(RedisModule_CallReplyProto)(RedisModuleCallReply *reply, size_t *len);
void REDISMODULE_API_FUNC(RedisModule_FreeCallReply)(RedisModuleCallReply *reply);
int REDISMODULE_API_FUNC(RedisModule_CallReplyType)(RedisModuleCallReply *reply);
long long REDISMODULE_API_FUNC(RedisModule_CallReplyInteger)(RedisModuleCallReply *reply);
size_t REDISMODULE_API_FUNC(RedisModule_CallReplyLength)(RedisModuleCallReply *reply);
RedisModuleCallReply *REDISMODULE_API_FUNC(RedisModule_CallReplyArrayElement)(RedisModuleCallReply *reply, size_t idx);
RedisModuleString *REDISMODULE_API_FUNC(RedisModule_CreateString)(RedisModuleCtx *ctx, const char *ptr, size_t len);
RedisModuleString *REDISMODULE_API_FUNC(RedisModule_CreateStringFromLongLong)(RedisModuleCtx *ctx, long long ll);
RedisModuleString *REDISMODULE_API_FUNC(RedisModule_CreateStringFromString)(RedisModuleCtx *ctx, const RedisModuleString *str);
RedisModuleString *REDISMODULE_API_FUNC(RedisModule_CreateStringPrintf)(RedisModuleCtx *ctx, const char *fmt, ...);
void REDISMODULE_API_FUNC(RedisModule_FreeString)(RedisModuleCtx *ctx, RedisModuleString *str);
const char *REDISMODULE_API_FUNC(RedisModule_StringPtrLen)(const RedisModuleString *str, size_t *len);
int REDISMODULE_API_FUNC(RedisModule_ReplyWithError)(RedisModuleCtx *ctx, const char *err);
int REDISMODULE_API_FUNC(RedisModule_ReplyWithSimpleString)(RedisModuleCtx *ctx, const char *msg);
int REDISMODULE_API_FUNC(RedisModule_ReplyWithArray)(RedisModuleCtx *ctx, long len);
void REDISMODULE_API_FUNC(RedisModule_ReplySetArrayLength)(RedisModuleCtx *ctx, long len);
int REDISMODULE_API_FUNC(RedisModule_ReplyWithStringBuffer)(RedisModuleCtx *ctx, const char *buf, size_t len);
int REDISMODULE_API_FUNC(RedisModule_ReplyWithString)(RedisModuleCtx *ctx, RedisModuleString *str);
int REDISMODULE_API_FUNC(RedisModule_ReplyWithNull)(RedisModuleCtx *ctx);
int REDISMODULE_API_FUNC(RedisModule_ReplyWithDouble)(RedisModuleCtx *ctx, double d);
int REDISMODULE_API_FUNC(RedisModule_ReplyWithCallReply)(RedisModuleCtx *ctx, RedisModuleCallReply *reply);
int REDISMODULE_API_FUNC(RedisModule_StringToLongLong)(const RedisModuleString *str, long long *ll);
int REDISMODULE_API_FUNC(RedisModule_StringToDouble)(const RedisModuleString *str, double *d);
void REDISMODULE_API_FUNC(RedisModule_AutoMemory)(RedisModuleCtx *ctx);
int REDISMODULE_API_FUNC(RedisModule_Replicate)(RedisModuleCtx *ctx, const char *cmdname, const char *fmt, ...);
int REDISMODULE_API_FUNC(RedisModule_ReplicateVerbatim)(RedisModuleCtx *ctx);
const char *REDISMODULE_API_FUNC(RedisModule_CallReplyStringPtr)(RedisModuleCallReply *reply, size_t *len);
RedisModuleString *REDISMODULE_API_FUNC(RedisModule_CreateStringFromCallReply)(RedisModuleCallReply *reply);
int REDISMODULE_API_FUNC(RedisModule_DeleteKey)(RedisModuleKey *key);
int REDISMODULE_API_FUNC(RedisModule_StringSet)(RedisModuleKey *key, RedisModuleString *str);
char *REDISMODULE_API_FUNC(RedisModule_StringDMA)(RedisModuleKey *key, size_t *len, int mode);
int REDISMODULE_API_FUNC(RedisModule_StringTruncate)(RedisModuleKey *key, size_t newlen);
mstime_t REDISMODULE_API_FUNC(RedisModule_GetExpire)(RedisModuleKey *key);
int REDISMODULE_API_FUNC(RedisModule_SetExpire)(RedisModuleKey *key, mstime_t expire);
int REDISMODULE_API_FUNC(RedisModule_ZsetAdd)(RedisModuleKey *key, double score, RedisModuleString *ele, int *flagsptr);
int REDISMODULE_API_FUNC(RedisModule_ZsetIncrby)(RedisModuleKey *key, double score, RedisModuleString *ele, int *flagsptr, double *newscore);
int REDISMODULE_API_FUNC(RedisModule_ZsetScore)(RedisModuleKey *key, RedisModuleString *ele, double *score);
int REDISMODULE_API_FUNC(RedisModule_ZsetRem)(RedisModuleKey *key, RedisModuleString *ele, int *deleted);
void REDISMODULE_API_FUNC(RedisModule_ZsetRangeStop)(RedisModuleKey *key);
int REDISMODULE_API_FUNC(RedisModule_ZsetFirstInScoreRange)(RedisModuleKey *key, double min, double max, int minex, int maxex);
int REDISMODULE_API_FUNC(RedisModule_ZsetLastInScoreRange)(RedisModuleKey *key, double min, double max, int minex, int maxex);
int REDISMODULE_API_FUNC(RedisModule_ZsetFirstInLexRange)(RedisModuleKey *key, RedisModuleString *min, RedisModuleString *max);
int REDISMODULE_API_FUNC(RedisModule_ZsetLastInLexRange)(RedisModuleKey *key, RedisModuleString *min, RedisModuleString *max);
RedisModuleString *REDISMODULE_API_FUNC(RedisModule_ZsetRangeCurrentElement)(RedisModuleKey *key, double *score);
int REDISMODULE_API_FUNC(RedisModule_ZsetRangeNext)(RedisModuleKey *key);
int REDISMODULE_API_FUNC(RedisModule_ZsetRangePrev)(RedisModuleKey *key);
int REDISMODULE_API_FUNC(RedisModule_ZsetRangeEndReached)(RedisModuleKey *key);
int REDISMODULE_API_FUNC(RedisModule_HashSet)(RedisModuleKey *key, int flags, ...);
int REDISMODULE_API_FUNC(RedisModule_HashGet)(RedisModuleKey *key, int flags, ...);
int REDISMODULE_API_FUNC(RedisModule_IsKeysPositionRequest)(RedisModuleCtx *ctx);
void REDISMODULE_API_FUNC(RedisModule_KeyAtPos)(RedisModuleCtx *ctx, int pos);
unsigned long long REDISMODULE_API_FUNC(RedisModule_GetClientId)(RedisModuleCtx *ctx);
int REDISMODULE_API_FUNC(RedisModule_GetContextFlags)(RedisModuleCtx *ctx);
void *REDISMODULE_API_FUNC(RedisModule_PoolAlloc)(RedisModuleCtx *ctx, size_t bytes);
RedisModuleType *REDISMODULE_API_FUNC(RedisModule_CreateDataType)(RedisModuleCtx *ctx, const char *name, int encver, RedisModuleTypeMethods *typemethods);
int REDISMODULE_API_FUNC(RedisModule_ModuleTypeSetValue)(RedisModuleKey *key, RedisModuleType *mt, void *value);
RedisModuleType *REDISMODULE_API_FUNC(RedisModule_ModuleTypeGetType)(RedisModuleKey *key);
void *REDISMODULE_API_FUNC(RedisModule_ModuleTypeGetValue)(RedisModuleKey *key);
void REDISMODULE_API_FUNC(RedisModule_SaveUnsigned)(RedisModuleIO *io, uint64_t value);
uint64_t REDISMODULE_API_FUNC(RedisModule_LoadUnsigned)(RedisModuleIO *io);
void REDISMODULE_API_FUNC(RedisModule_SaveSigned)(RedisModuleIO *io, int64_t value);
int64_t REDISMODULE_API_FUNC(RedisModule_LoadSigned)(RedisModuleIO *io);
void REDISMODULE_API_FUNC(RedisModule_EmitAOF)(RedisModuleIO *io, const char *cmdname, const char *fmt, ...);
void REDISMODULE_API_FUNC(RedisModule_SaveString)(RedisModuleIO *io, RedisModuleString *s);
void REDISMODULE_API_FUNC(RedisModule_SaveStringBuffer)(RedisModuleIO *io, const char *str, size_t len);
RedisModuleString *REDISMODULE_API_FUNC(RedisModule_LoadString)(RedisModuleIO *io);
char *REDISMODULE_API_FUNC(RedisModule_LoadStringBuffer)(RedisModuleIO *io, size_t *lenptr);
void REDISMODULE_API_FUNC(RedisModule_SaveDouble)(RedisModuleIO *io, double value);
double REDISMODULE_API_FUNC(RedisModule_LoadDouble)(RedisModuleIO *io);
void REDISMODULE_API_FUNC(RedisModule_SaveFloat)(RedisModuleIO *io, float value);
float REDISMODULE_API_FUNC(RedisModule_LoadFloat)(RedisModuleIO *io);
void REDISMODULE_API_FUNC(RedisModule_Log)(RedisModuleCtx *ctx, const char *level, const char *fmt, ...);
void REDISMODULE_API_FUNC(RedisModule_LogIOError)(RedisModuleIO *io, const char *levelstr, const char *fmt, ...);
int REDISMODULE_API_FUNC(RedisModule_StringAppendBuffer)(RedisModuleCtx *ctx, RedisModuleString *str, const char *buf, size_t len);
void REDISMODULE_API_FUNC(RedisModule_RetainString)(RedisModuleCtx *ctx, RedisModuleString *str);
int REDISMODULE_API_FUNC(RedisModule_StringCompare)(RedisModuleString *a, RedisModuleString *b);
RedisModuleCtx *REDISMODULE_API_FUNC(RedisModule_GetContextFromIO)(RedisModuleIO *io);
long long REDISMODULE_API_FUNC(RedisModule_Milliseconds)(void);
void REDISMODULE_API_FUNC(RedisModule_DigestAddStringBuffer)(RedisModuleDigest *md, unsigned char *ele, size_t len);
void REDISMODULE_API_FUNC(RedisModule_DigestAddLongLong)(RedisModuleDigest *md, long long ele);
void REDISMODULE_API_FUNC(RedisModule_DigestEndSequence)(RedisModuleDigest *md);

#ifdef REDISMODULE_EXPERIMENTAL_API

RedisModuleBlockedClient *REDISMODULE_API_FUNC(RedisModule_BlockClient)(RedisModuleCtx *ctx, RedisModuleCmdFunc reply_callback, RedisModuleCmdFunc timeout_callback, void (*free_privdata)(void*), long long timeout_ms);
int REDISMODULE_API_FUNC(RedisModule_UnblockClient)(RedisModuleBlockedClient *bc, void *privdata);
int REDISMODULE_API_FUNC(RedisModule_IsBlockedReplyRequest)(RedisModuleCtx *ctx);
int REDISMODULE_API_FUNC(RedisModule_IsBlockedTimeoutRequest)(RedisModuleCtx *ctx);
void *REDISMODULE_API_FUNC(RedisModule_GetBlockedClientPrivateData)(RedisModuleCtx *ctx);
int REDISMODULE_API_FUNC(RedisModule_AbortBlock)(RedisModuleBlockedClient *bc);
RedisModuleCtx *REDISMODULE_API_FUNC(RedisModule_GetThreadSafeContext)(RedisModuleBlockedClient *bc);
void REDISMODULE_API_FUNC(RedisModule_FreeThreadSafeContext)(RedisModuleCtx *ctx);
void REDISMODULE_API_FUNC(RedisModule_ThreadSafeContextLock)(RedisModuleCtx *ctx);
void REDISMODULE_API_FUNC(RedisModule_ThreadSafeContextUnlock)(RedisModuleCtx *ctx);

#endif

#endif
