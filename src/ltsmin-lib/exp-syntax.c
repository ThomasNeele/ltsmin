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

void collect_action_labels(exp_model_t model) {
    model->action_labels = SIcreate();
    for(int i = 0; i < model->num_sync_rules; i++) {
        SIput(model->action_labels, model->sync_rules[i][model->num_processes]);
    }
    for(int i = 0; i < model->num_processes; i++) {
        for(int j = 0; j < SIgetCount(model->processes[i].action_labels); j++) {
            int is_sync_action = 0;
            for (int k = 0; k < model->num_sync_rules; k++) {
                if (model->sync_rules[k][i] != NULL && strcmp(model->sync_rules[k][i], SIget(model->processes[i].action_labels, j))) {
                    is_sync_action = 1;
                    break;
                }
            }
            if(is_sync_action == 0) {
                SIput(model->action_labels, SIget(model->processes[i].action_labels, j));
            }
        }
    }
}

int num_ones(int a) {
    int result = 0;
    for(int i = 0; a != 0 && i < 32; i++) {
        if(a & 1) {
            result++;
        }
        a >>= 1;
    }
    return result;
}

char ***sync_actions_to_rules(list_t actions, exp_model_t model) {
    list_t current = actions;
    int process_index[model->num_processes];
    int num_processes_with_action = 0;
    list_t vector_list = NULL;
    while(current) {
        sync_action_number_t action = current->item;
        for(int i = 0; i < model->num_processes; i++) {
            if(SIlookup(model->processes[i].action_labels, action->label) != SI_INDEX_FAILED) {
                // Process contains this action
                process_index[num_processes_with_action] = i;
                num_processes_with_action++;
            }
        }
        for(int i = 0; i < (1 << num_processes_with_action); i++) {
            if(num_ones(i) == action->number) {
                char** sync_vector = RTmalloc(sizeof(char*) * (model->num_processes + 1));
                char*** sync_vector_address = RTmalloc(sizeof(char**));
                *sync_vector_address = sync_vector;
                for(int j = 0; j < num_processes_with_action; j++) {
                    sync_vector[process_index[j]] = (i >> j) & 1 ? action->label : NULL;
                }
                sync_vector[model->num_processes] = action->label;
                vector_list = expAddList(vector_list, sync_vector_address);
            }
        }
        list_t prev = current->prev;
        RTfree(current);
        current = prev;
    }
    model->num_sync_rules = expListLength(vector_list);
    return (char***) expListToArray(vector_list, sizeof(char**));
}
