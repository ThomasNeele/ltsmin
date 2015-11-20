#include <ltsmin-lib/exp-syntax.h>
#include <hre/user.h>
#include <ltsmin-lib/exp-lexer.h>

exp_parse_env_t EXPParseEnvCreate() {
    exp_parse_env_t result = RTmalloc(sizeof(struct exp_parse_env_s));
    result->strings = SIcreate();
    return result;
}

void EXPParseEnvDestroy(exp_parse_env_t env) {
    SIdestroy(&env->strings);
    RTfree(env);
}

exp_model_t exp_parse_stream(const char* filename) {
    yyscan_t scanner;
    exp_lex_init(&scanner);
    exp_model_t result = exp_recur_parse_stream(scanner, filename);
    exp_lex_destroy(scanner);
    return result;
}

exp_model_t exp_recur_parse_stream(void* scanner, const char* filename) {
    FILE *in = fopen( filename, "r" );
    if(in == NULL) {
        Abort("The referenced file could not be opened: %s", filename);
    }
    exp_parse_env_t env = EXPParseEnvCreate();
    env->parser = expParseAlloc(RTmalloc);
    env->scanner = scanner;
    env->filename = filename;
    exp_parse_env_t temp = exp_get_extra(scanner);
    exp_set_extra(env, scanner);

    YY_BUFFER_STATE buffer = exp__create_buffer(in, ENV_BUFFER_SIZE, scanner);
    exp_push_buffer_state(buffer, scanner);
    exp_lex( scanner );
    exp_pop_buffer_state(scanner);

    expParseFree( env->parser , RTfree );
    exp_model_t result = env->model;
    exp_set_extra(temp, scanner);
    fclose(in);
    EXPParseEnvDestroy(env);
    return result;
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

void **expListToArray(list_t list, size_t item_size) {
    unsigned int length = expListLength(list);
    char *result = RTmalloc(item_size * length);
    list_t current = list;
    int i = 0;
    list_t prev;
    while(current) {
        memcpy(result + (length - 1 - i) * item_size, current->item, item_size);
        prev = current->prev;
        RTfree(current->item);
        RTfree(current);
        current = prev;
        i++;
    }
    return (void*) result;
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