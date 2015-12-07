#include <math.h>

#include <hre/stringindex.h>
#include <hre/user.h>
#include <pins-lib/exp-pins.h>


static void exp_popt(poptContext con,
                     enum poptCallbackReason reason,
                     const struct poptOption *opt,
                     const char *arg, void *data){
    (void)con;(void)opt;(void)arg;(void)data;
    switch(reason){
        case POPT_CALLBACK_REASON_PRE:
            break;
        case POPT_CALLBACK_REASON_POST:
            GBregisterLoader("exp", EXPloadGreyboxModel);
            Warning(info,"EXP language module initialized");
            return;
        case POPT_CALLBACK_REASON_OPTION:
            break;
    }
    Abort("unexpected call to exp_popt");
}
struct poptOption exp_options[]= {
        {NULL, 0 , POPT_ARG_CALLBACK|POPT_CBFLAG_POST|POPT_CBFLAG_SKIPOPTION , exp_popt, 0 , NULL , NULL },
        POPT_TABLEEND
};

int determine_state_length(exp_model_t model, int *numNetworkNodes, int *numProcesses) {
    (*numNetworkNodes)++;
    if(model->num_processes == 0) {
        (*numProcesses)++;
        return (int) (log(model->process_states) / log(2));
    } else {
        int result = 0;
        for(int i = 0; i < model->num_processes; i++) {
            result += determine_state_length(&model->processes[i], numNetworkNodes, numProcesses);
        }
        return result;
    }
}

/*
int getInitialState(exp_model_t model, int *init_state, int offset_bits) {
    if(model->num_processes == 0) {
        int array_index = offset_bits / ((int) sizeof(int) * 8);
        int offset_element = offset_bits % ((int) sizeof(int) * 8);
        int numProcTemp;
        int sv_nbits = determine_state_length(model, &numProcTemp, &numProcTemp);
        if(offset_element + sv_nbits > 32) {
            // Split the bits of this process over two ints
            init_state[array_index] |= model->initial_state >> (offset_element + sv_nbits - (int) sizeof(int) * 8);
            init_state[array_index+1] |= model->initial_state <<  (2 * (int) sizeof(int) * 8 - offset_element - sv_nbits);
        } else {
            // Initial state fits in this int
            init_state[array_index] |= model->initial_state << ((int) sizeof(int) * 8 - offset_element - sv_nbits);
        }
        return offset_bits + sv_nbits;
    } else {
        int updated_offset = offset_bits;
        for(int i = 0; i < model->num_processes; i++) {
            updated_offset = getInitialState(&model->processes[i], init_state, updated_offset);
        }
        return updated_offset;
    }
}*/

char**SItoArray(string_index_t si) {
    int num_labels = SIgetCount(si);
    char** labels = RTmalloc(sizeof(char*) * num_labels);
    int index = 0;
    for(int i = 0; index < num_labels; i++) {
        if(SIget(si,index) != NULL) {
            labels[index] = SIget(si,index);
            index++;
        }
    }
    SIdestroy(&si);
    return labels;
}

void collect_edge_labels(exp_model_t model, int *processCounter, int *labelCount, char ***names) {
    string_index_t si = SIcreate();
    if(model->num_processes == 0) {
        // Put each label in the SI
        for(int i = 0; i < model->process_states; i++) {
            for(int j = 0; j < model->process_states; j++) {
                if(model->transitions[i][j] != NULL) {
                    SIput(si, model->transitions[i][j]);
                }
            }
        }
    } else {
        for(int i = 0; i < model->num_processes; i++) {
            collect_edge_labels(&model->processes[i], processCounter, labelCount, names);
        }
        // Postfix tree walking order: put the labels of the sync rules
        for(int i = 0; i < model->num_sync_rules; i++) {
            SIput(si, model->sync_rules[i][model->num_processes]);
        }
    }
    labelCount[*processCounter] = SIgetCount(si);
    names[*processCounter] = SItoArray(si);
    (*processCounter)++;
}

static exp_model_t flatten_network(exp_model_t model);
static exp_model_t flatten_network(exp_model_t model) {
    int is_flat = 1;
    for(int i = 0; i < model->num_processes; i++) {
        if(model->processes[i].num_processes != 0) {
            is_flat = 0;
            break;
        }
    }
    if(is_flat) {
        return model;
    }

    exp_model_t processes[model->num_processes];
    unsigned int num_processes = 0;
    for(int i = 0; i < model->num_processes; i++) {
        processes[i] = flatten_network(&model->processes[i]);
        num_processes += processes[i]->num_processes > 0 ? processes[i]->num_processes : 1;
    }

    exp_model_t flat_processes = RTmalloc(sizeof(struct exp_model_s) * num_processes);
    int process_counter = 0;
    for(int i = 0; i < model->num_processes; i++) {
        if(model->processes[i].num_processes == 0) {
            flat_processes[process_counter] = model->processes[i];
            process_counter++;
        } else {
            for (int j = 0; j < model->processes[i].num_processes; j++) {
                flat_processes[process_counter] = model->processes[i].processes[j];
                process_counter++;
            }
        }
    }

    char*** sync_vectors = NULL;
    unsigned int total_num_vectors = 0;
    for(int i = 0; i < model->num_sync_rules; i++) {
        char*** result_rule = RTmallocZero(sizeof(char**) * 1);
        result_rule[0] = RTmallocZero(sizeof(char*) * (num_processes + 1));
        result_rule[0][num_processes] = model->sync_rules[i][model->num_processes];
        int num_rules = 1;
        process_counter = 0;
        for(int j = 0; j < model->num_processes; j++) {
            int num_subprocesses = model->processes[j].num_processes > 0 ? model->processes[j].num_processes : 1;
            char* subvector[model->processes[j].num_sync_rules + num_subprocesses]
                           [num_subprocesses];
            int num_matches = 0;
            // See if the result of any of the sync rules of the subprocess matches
            for(int k = 0; k < model->processes[j].num_sync_rules; k++) {
                if(model->sync_rules[i][j] != NULL
                    && strcmp(model->sync_rules[i][j],
                             model->processes[j].sync_rules[k][model->processes[j].num_processes]) == 0) {
                    memcpy(subvector[num_matches], model->processes[j].sync_rules[k], sizeof(char*) * model->processes[j].num_processes);
                    num_matches++;
                }
            }
            // See if there is a matching transition in a subsubprocess that is not hidden by
            // a sync rule
            for(int k = 0; k < model->processes[j].num_processes; k++) {
                int process_has_local_action =
                        SIlookup(model->processes[j].processes[k].action_labels, model->sync_rules[i][j])
                            != SI_INDEX_FAILED;
                for(int l = 0; l < model->processes[j].num_sync_rules && process_has_local_action; l++) {
                    if(model->processes[j].sync_rules[l][k] != NULL) {
                        process_has_local_action = 0;
                    }
                }
                if(process_has_local_action) {
                    subvector[num_matches][k] = model->sync_rules[i][j];
                    num_matches++;
                }
            }
            // If the subprocess is not a network, see if it has the matching action
            if(model->processes[j].num_processes == 0) {
                int process_has_local_action =
                        SIlookup(model->processes[j].action_labels, model->sync_rules[i][j])
                        != SI_INDEX_FAILED;
                if(process_has_local_action) {
                    subvector[num_matches][0] = model->sync_rules[i][j];
                    num_matches++;
                }
            }
            // If there is more than 1 match, the result_rule array has to be expanded
            if(num_matches > 1) {
                result_rule = RTrealloc(result_rule, sizeof(char**) * num_rules * num_matches);
                for(int k = num_rules; k < num_matches * num_rules; k++) {
                    result_rule[k] = RTmalloc(sizeof(char*) * (num_processes + 1));
                    memcpy(result_rule[k], result_rule[k % num_rules], sizeof(char*) * (num_processes + 1));
                }
                num_rules *= num_matches;
            } else {
                num_matches = 1;
            }
            // Copy all the matches into the expanded result_rule array
            for(int k = 0; k < num_rules; k++) {
                memcpy(&result_rule[k][process_counter], subvector[k / num_matches],
                       sizeof(char*) * num_subprocesses);
            }
            process_counter += num_subprocesses;
        }
        sync_vectors = RTrealloc(sync_vectors, (total_num_vectors + num_rules) * sizeof(char**));
        memcpy(&sync_vectors[total_num_vectors], result_rule, num_rules * sizeof(char**));
        total_num_vectors += num_rules;
        RTfree(result_rule);
    }

    exp_model_t result = RTmalloc(sizeof(struct exp_model_s));
    result->num_processes = num_processes;
    result->processes = flat_processes;
    result->num_sync_rules = total_num_vectors;
    result->sync_rules = sync_vectors;
    result->action_labels = model->action_labels;
    return result;
}

void network_to_array(exp_model_t model, exp_model_t *processes, int *processCounter) {
    if(model->num_processes != 0) {
        for(int i = 0; i < model->num_processes; i++) {
            network_to_array(&model->processes[i], processes, processCounter);
        }
    }
    processes[*processCounter] = model;
    (*processCounter)++;
}

//typedef void(*TransitionCB)(void*context,transition_info_t*transition_info,int*dst,int*cpy);
/**
 * Return number of transitions found
 */
int exp_next_long(model_t self, int group, int *src, TransitionCB cb, void *user_context) {
    exp_model_t model = GBgetContext(self);
    transition_info_t ti = GB_TI(NULL, group);
    int dst[model->num_processes];
    int num_transitions = 0;

    if(group < model->num_processes) {
        for(int i = 0; i < model->processes[group].process_states; i++) {
            if(model->processes[group].transitions[src[group]][i] != NULL) {
                int is_local_transition = 1;
                for(int j = 0; j < model->num_sync_rules; j++) {
                    if(model->sync_rules[j][group] != NULL &&
                            strcmp(model->sync_rules[j][group],
                                   model->processes[group].transitions[src[group]][i]) == 0) {
                        is_local_transition = 0;
                        break;
                    }
                }
                if(is_local_transition) {
                    memcpy(dst, src, sizeof(int) * model->num_processes);
                    dst[group] = i;
                    cb(user_context, &ti, dst, NULL);
                    num_transitions++;
                }
            }
        }
    } else {
        int rule_number = group - model->num_processes;
        int is_applicable = 1;
        int process_counter = 0;
        int process_index[model->num_processes];
        int process_num_transitions[model->num_processes];
        int* target_states[model->num_processes];
        for(int i = 0; i < model->num_processes; i++) {
            if(model->sync_rules[rule_number][i] != NULL) {
                target_states[process_counter] = RTmalloc(sizeof(int) * model->processes[i].process_states);
                int num_enabled = 0;
                for(int j = 0; j < model->processes[i].process_states; j++) {
                    if(model->processes[i].transitions[src[i]][j] != NULL &&
                            strcmp(model->sync_rules[rule_number][i],
                                   model->processes[i].transitions[src[i]][j]) == 0) {
                        target_states[process_counter][num_enabled] = j;
                        num_enabled++;
                    }
                }
                if(num_enabled == 0) {
                    is_applicable = 0;
                    break;
                }
                process_index[process_counter] = i;
                process_num_transitions[process_counter] = num_enabled;
                process_counter++;
            }
        }
        if(is_applicable == 0) {
            for(int i = 0; i <= process_counter && i < model->num_processes; i++) {
                RTfree(target_states[i]);
            }
            return 0;
        }

        int trans_counter[process_counter + 1];
        for(int i = 0; i < process_counter + 1; i++) {
            trans_counter[i] = 0;
        }
        while(trans_counter[process_counter] == 0) {
            memcpy(dst, src, sizeof(int) * model->num_processes);
            for(int j = 0; j < process_counter; j++) {
                dst[process_index[j]] = target_states[j][trans_counter[j]];
            }
            cb(user_context, &ti, dst, NULL);
            num_transitions++;
            int i = 0;
            do {
                trans_counter[i] = (trans_counter[i] + 1) % process_num_transitions[i];
                i++;
            } while (trans_counter[i-1] == 0);
        }
    }
    return num_transitions;
}

void EXPloadGreyboxModel(model_t model, const char *filename) {
    exp_model_t exp_model = exp_parse_stream(filename);
    printf("Model has %d processes and %d sync rules\n", exp_model->num_processes, exp_model->num_sync_rules);
    if(exp_model->num_processes == 0) {
        printf("Process has %d states and %d transitions\n", exp_model->process_states, exp_model->process_transitions);
    }
    for(int i = 0; i < exp_model->num_processes; i++) {
        printf("Process %d has %d states and %d transitions\n", i, exp_model->processes[i].process_states, exp_model->processes[i].process_transitions);
    }

    exp_model = flatten_network(exp_model);

    lts_type_t ltstype = lts_type_create();
    lts_type_set_state_length(ltstype, exp_model->num_processes);

    lts_type_set_edge_label_count(ltstype, SIgetCount(exp_model->action_labels));

    int* init_state = RTmalloc(sizeof(int) * exp_model->num_processes);
    matrix_t* p_dm = (matrix_t*)RTmalloc(sizeof(matrix_t));
    dm_create(p_dm, exp_model->num_processes + exp_model->num_sync_rules, exp_model->num_processes);

    int is_new = 1;
    int stringType = lts_type_put_type(ltstype, "string", LTStypeChunk, &is_new);
    for(int i = 0; i < exp_model->num_processes; i++) {
        init_state[i] = exp_model->processes[i].initial_state;

        char *buf = RTmalloc(sizeof(char) * 20);
        snprintf(buf, 19, "[0,%d]", exp_model->processes[i].process_states - 1);

        is_new = 1;
        int typeno = lts_type_put_type(ltstype, buf, LTStypeRange, &is_new);
        if (is_new) {
            lts_type_set_range(ltstype, typeno, 0, exp_model->processes[i].process_states - 1);
        }
        lts_type_set_state_typeno(ltstype, i, typeno);
        buf = RTmalloc(sizeof(char) * 20);
        snprintf(buf, 19, "proc%d", i);
        lts_type_set_state_name(ltstype, i, buf);

        dm_set(p_dm, i, i);
        for(int j = 0; j < exp_model->num_sync_rules; j++) {
            if(exp_model->sync_rules[j][i] != NULL) {
                dm_set(p_dm, exp_model->num_processes + j, i);
            }
        }
    }
    for(int i = 0; i < SIgetCount(exp_model->action_labels); i++) {
        lts_type_set_edge_label_typeno(ltstype, i, stringType);
        lts_type_set_edge_label_name(ltstype, i, SIget(exp_model->action_labels, i));
    }

    GBsetContext(model,exp_model);
    GBsetLTStype(model,ltstype);

    GBsetDMInfo(model, p_dm);
    GBsetNextStateLong(model, exp_next_long);

    GBsetInitialState(model, init_state);
}
