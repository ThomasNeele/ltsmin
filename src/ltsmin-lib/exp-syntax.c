#include <ltsmin-lib/exp-syntax.h>
#include <hre/user.h>
#include "ltsmin-syntax.h"
#include "ltsmin-lexer.h"


exp_parse_env_t EXPParseEnvCreate() {
    exp_parse_env_t result = RTmalloc(sizeof(struct exp_parse_env_s));
    result->strings = SIcreate();
    return result;
}

void exp_parse_stream(int select, exp_parse_env_t env, stream_t stream) {
    yyscan_t scanner;
    env->input=stream;
    env->parser=ParseAlloc (RTmalloc);
    Parse(env->parser,select,0,env);
    ltsmin_lex_init_extra( env , &scanner );
    ltsmin_lex( scanner );
    ltsmin_lex_destroy(scanner);
    stream_close(&env->input);
    ParseFree( env->parser , RTfree );
}

list_t expAddList(list_t list, void* item) {
    list_t result = RTmalloc(sizeof(struct list_s));
    result->prev = list;
    result->item = item;
    return result;
}

unsigned int expListLength(list_t list) {
    list_t current = list;
    unsigned int result = 0;
    while(current) {
        current = current->prev;
        result += 1;
    }
    return result;
}

void** expListToArray(list_t list) {
    unsigned int length = expListLength(list);
    void **result = RTmalloc(sizeof(void*) * length);
    list_t current = list;
    int i = 0;
    list_t prev;
    while(current) {
        result[length - 1 - i] = current->item;
        prev = current->prev;
        RTfree(current);
        current = prev;
    }
    return result;
}

void expListFree(list_t list) {
    list_t current = list;
    list_t prev;
    while(current) {
        prev = current->prev;
        if(current->item) {
            RTfree(current->item);
        }
        RTfree(current);
        current = prev;
    }
}