#ifndef LTSMIN_EXP_SYNTAX_H
#define LTSMIN_EXP_SYNTAX_H

#include <hre-io/types.h>
#include <hre/stringindex.h>
#include <ltsmin-lib/exp-parse-env.h>
#include <stddef.h>
#include <stdio.h>

exp_parse_env_t EXPParseEnvCreate();
void EXPParseEnvDestroy(exp_parse_env_t env);
exp_model_t exp_parse_stream(const char* filename);
exp_model_t exp_recur_parse_stream(void* scanner, const char* filename);

list_t expAddList(list_t list, void* item);
unsigned int expListLength(list_t list) ;
void **expListToArray(list_t list, size_t item_size);
void expListFree(list_t list);

void collect_action_labels(exp_model_t model);
char*** sync_actions_to_rules(list_t actions, exp_model_t model);

extern void expParse(void*,int,int,exp_parse_env_t);
extern void expParseTrace(FILE* filename, char* prompt);
extern void *expParseAlloc(void *(*mallocProc)(size_t));
extern void expParseFree(void *p,void (*freeProc)(void*));
#endif //LTSMIN_EXP_SYNTAX_H
