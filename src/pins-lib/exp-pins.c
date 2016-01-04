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

static exp_model_t exp_flatten_network(exp_model_t model) {
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
        processes[i] = exp_flatten_network(&model->processes[i]);
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
            for(int ci = 0; ci < model->processes[j].num_sync_rules + num_subprocesses; ci++) {
                for(int cj = 0; cj < num_subprocesses; cj++) {
                    subvector[ci][cj] = NULL;
                }
            }
            int num_matches = 0;
            // Only do processsing when this process synchronizes
            if(model->sync_rules[i][j] != NULL) {
                // See if the result of any of the sync rules of the subprocess matches
                for (int k = 0; k < model->processes[j].num_sync_rules; k++) {
                    if (strcmp(model->sync_rules[i][j],
                               model->processes[j].sync_rules[k][model->processes[j].num_processes]) == 0) {
                        memcpy(subvector[num_matches], model->processes[j].sync_rules[k],
                               sizeof(char *) * model->processes[j].num_processes);
                        num_matches++;
                    }
                }
                // See if there is a matching transition in a subsubprocess that is not hidden by
                // a sync rule
                for (int k = 0; k < model->processes[j].num_processes; k++) {
                    int action_is_blocked =
                            SIlookup(model->processes[j].processes[k].action_labels, model->sync_rules[i][j])
                            == SI_INDEX_FAILED;
                    for (int l = 0; l < model->processes[j].num_sync_rules && !action_is_blocked; l++) {
                        if (model->processes[j].sync_rules[l][k] != NULL &&
                                strcmp(model->processes[j].sync_rules[l][k], model->sync_rules[i][j]) == 0) {
                            action_is_blocked = 1;
                        }
                    }
                    if (!action_is_blocked) {
                        subvector[num_matches][k] = model->sync_rules[i][j];
                        num_matches++;
                    }
                }
                // If the subprocess is not a network, see if it has the matching action
                if (model->processes[j].num_processes == 0) {
                    int process_has_local_action =
                            SIlookup(model->processes[j].action_labels, model->sync_rules[i][j])
                            != SI_INDEX_FAILED;
                    if (process_has_local_action) {
                        subvector[num_matches][0] = model->sync_rules[i][j];
                        num_matches++;
                    }
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
                memcpy(&result_rule[k][process_counter], subvector[k / (num_rules / num_matches)],
                       sizeof(char*) * num_subprocesses);
            }
            process_counter += num_subprocesses;
        }
        sync_vectors = RTrealloc(sync_vectors, (total_num_vectors + num_rules) * sizeof(char**));
        memcpy(&sync_vectors[total_num_vectors], result_rule, num_rules * sizeof(char**));
        total_num_vectors += num_rules;
        RTfree(result_rule);
    }

    // Look for synchronization rules in subprocesses
    // that do not take part in syncronization of this
    // model. They need to be copied directly
    process_counter = 0;
    list_t rule_list = NULL;
    for(int i = 0; i < model->num_processes; i++) {
        for(int j = 0; j < model->processes[i].num_sync_rules; j++) {
            int rule_is_blocked = 0;
            for(int k = 0; k < model->num_sync_rules; k++) {
                if(model->sync_rules[k][i] != NULL &&
                        strcmp(model->sync_rules[k][i],
                               model->processes[i].sync_rules[j][model->processes[i].num_processes]) == 0) {
                    rule_is_blocked = 1;
                    break;
                }
            }
            if(!rule_is_blocked) {
                char** new_rule = RTmallocZero(sizeof(char*) * (model->num_processes + 1));
                char*** new_rule_p = RTmalloc(sizeof(char**));
                *new_rule_p = new_rule;
                for(int k = 0; k < model->processes[i].num_processes; k++) {
                    new_rule[k + process_counter] = model->processes[i].sync_rules[j][k];
                }
                new_rule[num_processes] = model->processes[i].sync_rules[j][model->processes[i].num_processes];
                rule_list = expAddList(rule_list, new_rule_p);
            }
        }
        process_counter += model->processes[i].num_processes == 0 ? 1 : model->processes[i].num_processes;
    }
    int num_new_rules = expListLength(rule_list);
    if(num_new_rules > 0) {
        char ***unblocked_rules = (char ***) expListToArray(rule_list, sizeof(char **));
        sync_vectors = RTrealloc(sync_vectors, (total_num_vectors + num_new_rules) * sizeof(char **));
        memcpy(&sync_vectors[total_num_vectors], unblocked_rules, num_new_rules * sizeof(char**));
        total_num_vectors += num_new_rules;
        RTfree(unblocked_rules);
    }

    exp_model_t result = RTmalloc(sizeof(struct exp_model_s));
    result->num_processes = num_processes;
    result->processes = flat_processes;
    result->num_sync_rules = total_num_vectors;
    result->sync_rules = sync_vectors;
    result->action_labels = model->action_labels;
    return result;
}

int exp_matching_offers(char *a, char *b) {
    if(a == NULL || b == NULL) {
        return 0;
    } else {
        char* offer_a = strpbrk(a, "!? (\t\x1f");
        char* offer_b = strpbrk(b, "!? (\t\x1f");
        if((offer_a == NULL || offer_a[0] == '\x1f') && (offer_b == NULL || offer_b[0] == '\x1f')) {
            return 1;
        } else if((offer_a == NULL || offer_a[0] == '\x1f') || (offer_b == NULL || offer_b[0] == '\x1f')) {
            return 0;
        } else {
            size_t offer_len_a = (size_t) strchr(offer_a, '\x1f');
            size_t offer_len_b = (size_t) strchr(offer_b, '\x1f');
            offer_len_a = offer_len_a == 0 ? strlen(a) : offer_len_a - (size_t) a;
            offer_len_b = offer_len_b == 0 ? strlen(b) : offer_len_b - (size_t) b;
            return offer_len_a == offer_len_b && strncmp(a, b, offer_len_a) == 0;
        }
    }
}

int exp_matching_gate(char* a, char* b) {
    if(a == NULL || b == NULL) {
        return 0;
    } else {
        size_t gate_length_a = (size_t) strpbrk(a, "!? (\t\x1f");
        size_t gate_length_b = (size_t) strpbrk(b, "!? (\t\x1f");
        gate_length_a = gate_length_a == 0 ? strlen(a) : gate_length_a - (size_t) a;
        gate_length_b = gate_length_b == 0 ? strlen(b) : gate_length_b - (size_t) b;
        return gate_length_a == gate_length_b && strncmp(a,b,gate_length_a) == 0;
    }
}

int exp_next_label(char** label) {
    *label = strchr(*label, '\x1f');
    if(*label) {
        (*label)++;
    }
}

/**
 * Return number of transitions found
 */
int exp_next_long(model_t self, int group, int *src, TransitionCB cb, void *user_context) {
    exp_model_t model = GBgetContext(self);
    transition_info_t ti = GB_TI(NULL, group);
    int dst[model->num_processes];
    int num_transitions = 0;

    if(group < model->num_processes) {
        if(model->processes[group].transitions[src[group]] != NULL) {
            for (int i = 0; i < model->processes[group].process_states; i++) {
                if (model->processes[group].transitions[src[group]][i] != NULL) {
                    char* label = model->processes[group].transitions[src[group]][i];
                    while(label) {
                        int is_local_transition = 1;
                        for (int j = 0; j < model->num_sync_rules; j++) {
                            if (exp_matching_gate(model->sync_rules[j][group],
                                                  label)) {
                                is_local_transition = 0;
                                break;
                            }
                        }
                        if (is_local_transition) {
                            memcpy(dst, src, sizeof(int) * model->num_processes);
                            dst[group] = i;
                            cb(user_context, &ti, dst, NULL);
                            num_transitions++;
                            break;
                        }
                        exp_next_label(&label);
                    }
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
                if(model->processes[i].transitions[src[i]] != NULL) {
                    for (int j = 0; j < model->processes[i].process_states; j++) {
                        char* label = model->processes[i].transitions[src[i]][j];
                        while(label) {
                            if (exp_matching_gate(model->sync_rules[rule_number][i],
                                                  label)) {
                                target_states[process_counter][num_enabled] = j;
                                num_enabled++;
                                break;
                            }
                            exp_next_label(&label);
                        }
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
            int matching_offers_exist = 0;
            char*reference_label = model->processes[process_index[0]]
                            .transitions[src[process_index[0]]][target_states[0][trans_counter[0]]];
            while(reference_label) {
                int all_processes_match = 1;
                if(!exp_matching_gate(model->sync_rules[rule_number][process_index[0]],
                            reference_label)) {
                    // This is not the gate that matched the sync rule
                    exp_next_label(&reference_label);
                    continue;
                }
                for (int i = 1; i < process_counter; i++) {
                    char* label = model->processes[process_index[i]]
                            .transitions[src[process_index[i]]][target_states[i][trans_counter[i]]];
                    int this_process_matches = 0;
                    while(label) {
                        if(!exp_matching_gate(model->sync_rules[rule_number][process_index[i]], label)) {
                            // This is not the gate you're looking for
                            exp_next_label(&label);
                            continue;
                        }
                        if (exp_matching_offers(reference_label, label)) {
                            this_process_matches = 1;
                            break;
                        }
                        exp_next_label(&label);
                    }
                    if(!this_process_matches) {
                        all_processes_match = 0;
                        break;
                    }
                }
                if(all_processes_match) {
                    matching_offers_exist = 1;
                    break;
                }
                exp_next_label(&reference_label);
            }
            if(matching_offers_exist) {
                memcpy(dst, src, sizeof(int) * model->num_processes);
                for (int j = 0; j < process_counter; j++) {
                    dst[process_index[j]] = target_states[j][trans_counter[j]];
                }
                cb(user_context, &ti, dst, NULL);
                num_transitions++;
            }
            int i = 0;
            do {
                trans_counter[i] = (trans_counter[i] + 1) % (i < process_counter ? process_num_transitions[i] : 2);
                i++;
            } while (trans_counter[i-1] == 0);
        }
    }
    return num_transitions;
}

void EXPloadGreyboxModel(model_t model, const char *filename) {
    exp_model_t exp_model = exp_parse_stream(filename);
    exp_model = exp_flatten_network(exp_model);

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
