#include <hre/config.h>
#include <ltsmin-lib/exp-syntax.h>
#include <hre/user.h>
#include <ltsmin-lib/exp-lexer.h>

exp_parse_env_t EXPParseEnvCreate() {
    exp_parse_env_t result = RTmalloc(sizeof(struct exp_parse_env_s));
    result->strings = SIcreate();
    return result;
}

void EXPParseEnvDestroy(exp_parse_env_t env) {
//    SIdestroy(&env->strings);
    RTfree(*(void**)((char*)env->strings + sizeof(array_manager_t) + 2*sizeof(int)));
    RTfree(*(void**)((char*)env->strings + sizeof(array_manager_t) + 2*sizeof(int) + sizeof(int*)));
    RTfree(*(void**)((char*)env->strings + sizeof(array_manager_t) + 2*sizeof(int) + 2*sizeof(int*)));
    RTfree(*(void**)((char*)env->strings + sizeof(array_manager_t) + 2*sizeof(int) + 2*sizeof(int*) + sizeof(char**)));
//    RTfree((void*)(env->strings + sizeof(array_manager_t) + 2*sizeof(int) + sizeof(int*)));
//    RTfree((void*)(env->strings + sizeof(array_manager_t) + 2*sizeof(int) + 2*sizeof(int*)));
//    RTfree((void*)(env->strings + sizeof(array_manager_t) + 2*sizeof(int) + 2*sizeof(int*) + sizeof(char**)));
    RTfree(env->strings);
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

void exp_collect_action_labels(exp_model_t model) {
    model->action_labels = SIcreate();
//    for(int i = 0; i < model->num_sync_rules; i++) {
//        SIput(model->action_labels, model->sync_rules[i][model->num_processes]);
//    }
    for(int i = 0; i < model->num_processes; i++) {
        for(int j = 0; j < SIgetCount(model->processes[i].action_labels); j++) {
//            int is_sync_action = 0;
//            for (int k = 0; k < model->num_sync_rules; k++) {
//                if (model->sync_rules[k][i] != NULL && strcmp(model->sync_rules[k][i], SIget(model->processes[i].action_labels, j)) == 0) {
//                    is_sync_action = 1;
//                    break;
//                }
//            }
//            if(is_sync_action == 0) {
                SIput(model->action_labels, SIget(model->processes[i].action_labels, j));
//            }
        }
    }
}

void exp_collect_gates(exp_model_t model) {
    model->action_gates = SIcreate();
    for(int i = 0; i < SIgetCount(model->action_labels); i++) {
        SIput(model->action_gates, exp_get_gate(SIget(model->action_labels, i)));
    }
}

char*exp_get_gate(char *label) {
    int i = 0;
    while(label[i] != '!' && label[i] != '?' && label[i] != ' ' && label[i] != '\t' && label[i] != '(' && label[i] != '\0') {
        i++;
    }
    char* result = RTmalloc(sizeof(char) * (i+1));
    strncpy(result, label, (size_t) i);
    result[i] = '\0';
    return result;
}

char* exp_get_offers(char* label) {
    char* offers = strpbrk(label, "!? (\t");
    if(offers == NULL) {
        offers = "";
    }
    return offers;
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

char ***exp_sync_actions_to_rules(list_t actions, exp_model_t model) {
    list_t current = actions;
    int process_index[model->num_processes];
    list_t vector_list = NULL;
    int num_processes_with_action = 0;
    while(current) {
        sync_action_number_t action = current->item;
        for(int i = 0; i < model->num_processes; i++) {
            if(SIlookup(model->processes[i].action_gates, action->label) != SI_INDEX_FAILED) {
                // Process contains this action
                process_index[num_processes_with_action] = i;
                num_processes_with_action++;
            }
        }
        if(action->number > num_processes_with_action) {
            Abort("Sync rule \"%s\" cannot be applied", action->label);
        }
        for(int i = 0; i < (1 << num_processes_with_action); i++) {
            if(num_ones(i) == action->number) {
                char** sync_vector = RTmallocZero(sizeof(char*) * (model->num_processes + 1));
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
        num_processes_with_action = 0;

    }
    model->num_sync_rules = expListLength(vector_list);
    return (char***) expListToArray(vector_list, sizeof(char**));
}

void exp_print_model(FILE *stream, exp_model_t model, int indent_level) {
    char tabs[indent_level + 1];
    for(int i = 0; i < indent_level; i++ ) {
        tabs[i] = '\t';
    }
    tabs[indent_level] = '\0';

    if(model->num_processes == 0) {
        fprintf(stream, tabs);
        fprintf(stream, "des (%d, %d, %d)\n", model->initial_state, model->process_transitions, model->process_states);
        for(int i = 0; i < model->process_states; i++) {
            if(model->transitions[i]) {
                for (int j = 0; j < model->transitions[i]->num; j++) {
                    fprintf(stream, tabs);
                    fprintf(stream, "\t(%d, \"%s\", %d)\n", i, model->transitions[i]->label[j],
                            model->transitions[i]->dest[j]);
                }
            }
        }
    } else {
        fprintf(stream, tabs);
        fprintf(stream, "par using\n");
        for (int i = 0; i < model->num_sync_rules; i++) {
            fprintf(stream, "%s\t", tabs);
            for (int j = 0; j < model->num_processes + 1; j++) {
                fprintf(stream, "%s ", model->sync_rules[i][j] != NULL ? model->sync_rules[i][j] : "_");
                if (j < model->num_processes - 1) {
                    fprintf(stream, "* ");
                } else if (j < model->num_processes) {
                    fprintf(stream, "-> ");
                } else if (i < model->num_sync_rules - 1){
                    fprintf(stream, ",");
                }
            }
            fprintf(stream, "\n");
        }
        fprintf(stream, tabs);
        fprintf(stream, "in\n");
        for (int i = 0; i < model->num_processes; i++) {
            exp_print_model(stream, &model->processes[i], indent_level + 1);
            if (i < model->num_processes - 1) {
                fprintf(stream, tabs);
                fprintf(stream, "||\n");
            }
        }
        fprintf(stream, tabs);
        fprintf(stream, "end par\n");
    }
}