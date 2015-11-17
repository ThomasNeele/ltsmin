#ifndef LTSMIN_EXP_SYNTAX_H
#define LTSMIN_EXP_SYNTAX_H

#include <hre-io/types.h>
#include <hre/stringindex.h>
#include <ltsmin-lib/exp-parse-env.h>
#include <stddef.h>

exp_parse_env_t EXPParseEnvCreate();
void exp_parse_stream(int select, exp_parse_env_t env, stream_t stream);

list_t expAddList(list_t list, void* item);
unsigned int expListLength(list_t list) ;
void** expListToArray(list_t list);
void expListFree(list_t list);

extern void expParse(void*,int,int,exp_parse_env_t);
extern void *expParseAlloc(void *(*mallocProc)(size_t));
extern void expParseFree(void *p,void (*freeProc)(void*));
#endif //LTSMIN_EXP_SYNTAX_H
