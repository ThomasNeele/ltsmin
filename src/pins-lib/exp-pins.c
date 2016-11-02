#include <hre/config.h>
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

typedef struct exp_int_trans_s {
    int num;
    int* label;
    int* dest;
} *exp_int_trans_t ;

typedef struct exp_pins_ctx_s {
    exp_model_t       model;
    /** Collection of syncing gates for each process */
    string_index_t*   sync_gates;
    /** Offset of each sync rule in the sequence of groups
     *  from gate based sync rule to concrete sync rule offset
     */
    int*              group_offset;
    /** From concrete sync rule offset to gate based
     *  sync rule offset
     */
    int*              label_index;
    int               num_labels;
    /** Collection of offers for each sync rule */
    string_index_t*   sync_offers;
    /** Transitions with integer labels */
    exp_int_trans_t**  trans_int;
    /** Sync rules with integer labels */
    int**              sync_rules_int;
    /** One bitvector per process to indicate which actions are local*/
    bitvector_t*       sync_actions;
} *exp_pins_ctx_t ;

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
        if(processes[i]->num_processes == 0) {
            memcpy(&flat_processes[process_counter], processes[i], sizeof(struct exp_model_s));
            process_counter++;
        } else {
            for (int j = 0; j < processes[i]->num_processes; j++) {
                memcpy(&flat_processes[process_counter], &processes[i]->processes[j], sizeof(struct exp_model_s));
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
                            SIlookup(model->processes[j].processes[k].action_gates, model->sync_rules[i][j])
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
                            SIlookup(model->processes[j].action_gates, model->sync_rules[i][j])
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
                char** new_rule = RTmallocZero(sizeof(char*) * (num_processes + 1));
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
    result->action_gates = model->action_gates;
    return result;
}

static int exp_matching_offers(char *a, char *b) {
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
            offer_len_a = offer_len_a == 0 ? strlen(offer_a) : offer_len_a - (size_t) offer_a;
            offer_len_b = offer_len_b == 0 ? strlen(offer_b) : offer_len_b - (size_t) offer_b;
            return offer_len_a == offer_len_b && strncmp(offer_a, offer_b, offer_len_a) == 0;
        }
    }
}

static int exp_matching_gate(char* a, char* b) {
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

static void exp_next_label(char** label) {
    *label = strchr(*label, '\x1f');
    if(*label) {
        (*label)++;
    }
}

/**
 * Return number of transitions found
 */
int exp_next_long(model_t self, int group, int *src, TransitionCB cb, void *user_context) {
    exp_pins_ctx_t context = GBgetContext(self);
    exp_model_t model = context->model;
    transition_info_t ti = GB_TI(NULL, group);
    int dst[model->num_processes];
    int num_transitions = 0;

    if(group < model->num_processes) {
        exp_int_trans_t trans = context->trans_int[group][src[group]];
        for (int i = 0; trans != NULL && i < trans->num; i++) {
            int label = trans->label[i];
            if(bitvector_is_set(&context->sync_actions[group], (size_t) label) == 0) {
                memcpy(dst, src, sizeof(int) * model->num_processes);
                dst[group] = trans->dest[i];
                cb(user_context, &ti, dst, NULL);
                num_transitions++;
            }
        }
    } else {
        int rule_number = group - model->num_processes;
        int is_applicable = 1;
        int process_counter = 0;
        int process_index[model->num_processes];
        int process_num_transitions[model->num_processes];
        int max_succ = 0;
        for(int i = 0; i < model->num_processes; i++) {
            if (context->sync_rules_int[rule_number][i] != -1 &&
                context->trans_int[i][src[i]] != NULL &&
                max_succ < context->trans_int[i][src[i]]->num) {
                max_succ = context->trans_int[i][src[i]]->num;
            }
        }
        int target_states[model->num_processes][max_succ];
        for(int i = 0; i < model->num_processes; i++) {
            if(context->sync_rules_int[rule_number][i] != -1) {
                if(context->trans_int[i][src[i]] == NULL) {
                    // This state is a deadlock
                    is_applicable = 0;
                    break;
                }
                int num_enabled = 0;
                exp_int_trans_t trans =  context->trans_int[i][src[i]];
                for (int j = 0; j < trans->num; j++) {
                    int label = trans->label[j];
                    if(label == context->sync_rules_int[rule_number][i]) {
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
            return 0;
        }

        int trans_counter[process_counter + 1];
        for(int i = 0; i < process_counter + 1; i++) {
            trans_counter[i] = 0;
        }
        while(trans_counter[process_counter] == 0) {
            memcpy(dst, src, sizeof(int) * model->num_processes);
            for (int j = 0; j < process_counter; j++) {
                dst[process_index[j]] = context->trans_int[process_index[j]]
                        [src[process_index[j]]]->dest[target_states[j][trans_counter[j]]];
            }
            cb(user_context, &ti, dst, NULL);
            num_transitions++;
            int i = 0;
            do {
                trans_counter[i] = (trans_counter[i] + 1) % (i < process_counter ? process_num_transitions[i] : 2);
                i++;
            } while (trans_counter[i-1] == 0);
        }
    }
    return num_transitions;
}

static void print_string_index(string_index_t si) {
    printf("{");
    for(int i = 0; i < SIgetCount(si); i++) {
        printf("%s, ", SIget(si, i));
    }
    printf("}\n");
}

int exp_label_long(model_t self, int label, int* src) {
    exp_pins_ctx_t context = GBgetContext(self);
    exp_model_t model = context->model;
    if(label < model->num_processes) {
        exp_int_trans_t trans = context->trans_int[label][src[label]];
        for(int i = 0; trans != NULL && i < trans->num; i++) {
            int trans_label = trans->label[i];
            if(bitvector_is_set(&context->sync_actions[label], (size_t) trans_label) == 0) {
                // Found a local action
                return 1;
            }
        }
    } else {
        label -= model->num_processes;
        int proc_nr = label % model->num_processes;
        int rule_nr = label / model->num_processes;
        exp_int_trans_t trans = context->trans_int[proc_nr][src[proc_nr]];
        for(int i = 0; trans != NULL && i < trans->num; i++) {
            int trans_label = trans->label[i];
            if(trans_label == context->sync_rules_int[rule_nr][proc_nr]) {
                // Found a matching sync action
                return 1;
            }
        }
    }
    return 0;
}

static void exp_get_labels_group(model_t self,sl_group_enum_t group, int*src,int *label) {
    exp_pins_ctx_t context = GBgetContext(self);
    exp_model_t model = context->model;
    if (group == GB_SL_GUARDS || group == GB_SL_ALL) {
        for (int i = 0; i < context->num_labels; i++) {
            label[i] = exp_label_long(self,i,src);
        }
    }
}

static string_index_t* exp_collect_sync_offers(exp_model_t model) {
    string_index_t* sync_offers = RTmalloc(sizeof(string_index_t) * model->num_sync_rules);
    for(int i = 0; i < model->num_sync_rules; i++) {
        sync_offers[i] = SIcreate();
        for(int j = 0; j < model->num_processes; j++) {
            for(int k = 0; k < SIgetCount(model->processes[j].action_labels); k++) {
                if(exp_matching_gate(model->sync_rules[i][j], SIget(model->processes[j].action_labels, k))) {
                    SIput(sync_offers[i], exp_get_offers(SIget(model->processes[j].action_labels, k)));
                }
            }
        }
    }
    return sync_offers;
}

static void exp_action_to_group_guard(exp_pins_ctx_t context, char* action, int process, int to_guard, int* result) {
    exp_model_t model = context->model;
    if(SIlookup(context->sync_gates[process], exp_get_gate(action)) != SI_INDEX_FAILED) {
        // Action is a sync action
        int r_id = 0;
        for(int i = 0; i < model->num_sync_rules; i++) {
            if(exp_matching_gate(model->sync_rules[i][process], action)) {
                int gate_index = SIlookup(context->sync_offers[i], exp_get_offers(action));
                if(gate_index != SI_INDEX_FAILED) {
                    if(to_guard) {
                        result[r_id++] = model->num_processes + (context->group_offset[i] + gate_index) * model->num_processes + process;
                    } else {
                        result[r_id++] = model->num_processes + context->group_offset[i] + gate_index;
                    }
                }
            }
        }
        result[r_id] = -1;
    } else {
        // Action is a local action
        result[0] = process;
        result[1] = -1;
    }
}

static int guard_to_group(exp_pins_ctx_t context, int guard) {
    exp_model_t model = context->model;
    if(guard >= model->num_processes) {
        return (guard - model->num_processes) / model->num_processes + model->num_processes;
    } else {
        return guard;
    }
}

static void make_context_trans(exp_model_t model, exp_pins_ctx_t context) {
    exp_int_trans_t** trans = RTmalloc(model->num_processes * sizeof(exp_int_trans_t*));
    for(int i = 0; i < model->num_processes; i++) {
        trans[i] = RTmalloc(model->processes[i].process_states * sizeof(exp_int_trans_t));
        for(int j = 0; j < model->processes[i].process_states; j++) {
            trans[i][j] = RTmallocZero(sizeof(struct exp_int_trans_s));
            int array_size = 0;
            for(int k = 0; model->processes[i].transitions[j] != NULL && k < model->processes[i].transitions[j]->num; k++) {
                char* trans_label = model->processes[i].transitions[j]->label[k];
                while(trans_label) {
                    char* next_label = strpbrk(trans_label,"\x1f");
                    char tmp;
                    if(next_label != NULL) {
                        tmp = next_label[0];
                        next_label[0] = '\0';
                    }
                    if(trans[i][j]->num + 1 > array_size) {
                        array_size = array_size == 0 ? 1 : array_size << 1;
                        trans[i][j]->label = RTrealloc(trans[i][j]->label, array_size * sizeof(int));
                        trans[i][j]->dest = RTrealloc(trans[i][j]->dest, array_size * sizeof(int));
                    }
                    trans[i][j]->label[trans[i][j]->num] = SIlookup(model->action_labels, trans_label);
                    trans[i][j]->dest[trans[i][j]->num] = model->processes[i].transitions[j]->dest[k];
                    trans[i][j]->num++;
                    if(next_label != NULL) {
                        next_label[0] = tmp;
                    }
                    exp_next_label(&trans_label);
                }
            }
        }
    }
    context->trans_int = trans;

    char *buf = RTmalloc(sizeof(char) * 256);
    int** new_sync_rules = RTmalloc(sizeof(int*) * context->group_offset[model->num_sync_rules]);
    bitvector_t* sync_actions = RTmalloc(sizeof(bitvector_t) * model->num_processes);
    for(int i = 0; i < model->num_processes; i++) {
        bitvector_create(&sync_actions[i], (size_t) SIgetCount(model->action_labels));
    }
    for(int i = 0; i < context->group_offset[model->num_sync_rules]; i++) {
        new_sync_rules[i] = RTmalloc(sizeof(int) * model->num_processes);
        for(int j = 0; j < model->num_processes; j++) {
            if(model->sync_rules[context->label_index[i]][j] != NULL) {
                snprintf(buf, 256, "%s%s", model->sync_rules[context->label_index[i]][j],
                         SIget(context->sync_offers[context->label_index[i]], i - context->group_offset[context->label_index[i]]));
                new_sync_rules[i][j] = SIlookup(model->action_labels, buf);
                bitvector_set(&sync_actions[j], (size_t) new_sync_rules[i][j]);
            } else {
                new_sync_rules[i][j] = -1;
            }
        }
    }
    context->sync_rules_int = new_sync_rules;
    context->sync_actions = sync_actions;
}

void EXPloadGreyboxModel(model_t model, const char *filename) {
    exp_model_t exp_model = exp_parse_stream(filename);
    exp_model = exp_flatten_network(exp_model);

    exp_pins_ctx_t context = RTmalloc(sizeof(struct exp_pins_ctx_s));
    context->sync_offers = exp_collect_sync_offers(exp_model);
    context->group_offset = RTmalloc(sizeof(int) * (exp_model->num_sync_rules + 1));
    int offset = 0;
    for(int i = 0; i < exp_model->num_sync_rules; i++) {
        context->group_offset[i] = offset;
        offset += SIgetCount(context->sync_offers[i]);
    }
    context->group_offset[exp_model->num_sync_rules] = offset;
    int num_groups = offset + exp_model->num_processes;
    int num_sync_groups = offset;
    int num_labels = exp_model->num_processes + num_sync_groups * exp_model->num_processes;
    context->num_labels = num_labels;

    offset = 0;
    context->label_index = RTmalloc(sizeof(int) * (num_sync_groups+1));
    for(int i = 0; i < num_sync_groups; i++) {
        while(context->group_offset[offset+1] <= i) {
            offset++;
        }
        context->label_index[i] = offset;
    }
    context->label_index[num_sync_groups] = exp_model->num_sync_rules;
    make_context_trans(exp_model, context);

    lts_type_t ltstype = lts_type_create();
    lts_type_set_state_length(ltstype, exp_model->num_processes);
    // Each process has a guard for each action
    lts_type_set_state_label_count(ltstype, num_labels);
    lts_type_set_edge_label_count(ltstype, 0);

    int* init_state = RTmalloc(sizeof(int) * exp_model->num_processes);
    matrix_t* p_dm = RTmalloc(sizeof(matrix_t));
    dm_create(p_dm, num_groups, exp_model->num_processes);
    matrix_t* p_sl = RTmalloc(sizeof(matrix_t));
    dm_create(p_sl, num_labels, exp_model->num_processes);

    char *buf = RTmalloc(sizeof(char) * 256);
    int bool_is_new, bool_type = lts_type_put_type (ltstype, "bool", LTStypeDirect, &bool_is_new);
    int pc_is_new, pc_typeno = lts_type_put_type(ltstype, "pc", LTStypeDirect, &pc_is_new);
    for(int i = 0; i < exp_model->num_processes; i++) {
        init_state[i] = exp_model->processes[i].initial_state;

        snprintf(buf, 16, "pc%d", i);
        lts_type_set_state_name(ltstype, i, buf);
        lts_type_set_state_typeno(ltstype, i, pc_typeno);
        snprintf(buf, 32, "proc%d.local", i);
        lts_type_set_state_label_name(ltstype, i, buf);
        lts_type_set_state_label_typeno(ltstype, i, bool_type);

        dm_set(p_dm, i, i);
        dm_set(p_sl, i, i);
        for(int j = 0; j < num_sync_groups; j++) {
            if(exp_model->sync_rules[context->label_index[j]][i] != NULL) {
                dm_set(p_dm, exp_model->num_processes + j, i);
                dm_set(p_sl, exp_model->num_processes + j * exp_model->num_processes + i, i);
            }
        }
    }

    guard_t** guards = RTmalloc(sizeof(guard_t*) * num_labels);
    for(int i = 0; i < exp_model->num_processes; i++) {
        guard_t* guard = RTmalloc(sizeof(struct guard) + sizeof(int));
        guard->count = 1;
        guard->guard[0] = i;
        guards[i] = guard;
    }
    for(int i = 0; i < num_sync_groups; i++) {
        guard_t* guard = RTmallocZero(sizeof(struct guard) + sizeof(int) * exp_model->num_processes);
        guard->count = 0;
        int rule_nr = context->label_index[i];
        int offer_nr = i - context->group_offset[rule_nr];
        for(int j = 0; j < exp_model->num_processes; j++) {
            if(exp_model->sync_rules[rule_nr][j]) {
                guard->guard[guard->count] = exp_model->num_processes + i * exp_model->num_processes + j;
                guard->count++;
            }

            snprintf(buf, 256, "proc%d.act %s%s", j, exp_model->sync_rules[rule_nr][j], SIget(context->sync_offers[rule_nr], offer_nr));
            lts_type_set_state_label_name(ltstype, exp_model->num_processes + i*exp_model->num_processes + j, buf);
            lts_type_set_state_label_typeno(ltstype, exp_model->num_processes + i*exp_model->num_processes + j, bool_type);
        }
        guards[i + exp_model->num_processes] = guard;
    }
    GBsetGuardsInfo(model, guards);
    int* visibility = RTmallocZero(sizeof(int) * num_groups);
    GBsetPorGroupVisibility(model, visibility);
    visibility = RTmallocZero(sizeof(int) * num_labels);
    GBsetPorStateLabelVisibility(model, visibility);

    sl_group_t* sl_group_all = RTmallocZero(sizeof(sl_group_t) + num_labels * sizeof(int));
    sl_group_all->count = num_labels;
    for(int i=0; i < sl_group_all->count; i++) sl_group_all->sl_idx[i] = i;
    sl_group_t* sl_group_guards = RTmallocZero(sizeof(sl_group_t) + num_labels * sizeof(int));
    sl_group_guards->count = num_labels;
    for(int i=0; i < sl_group_guards->count; i++) sl_group_guards->sl_idx[i] = i;
    GBsetStateLabelGroupInfo(model, GB_SL_ALL, sl_group_all);
    GBsetStateLabelGroupInfo(model, GB_SL_GUARDS, sl_group_all);
    GBsetStateLabelsGroup(model, exp_get_labels_group);

    GBsetLTStype(model,ltstype);

    GBsetDMInfo(model, p_dm);
    GBsetStateLabelInfo(model, p_sl);
    GBsetNextStateLong(model, exp_next_long);
    GBsetStateLabelLong(model, exp_label_long);

    GBsetInitialState(model, init_state);

    context->model = exp_model;
    string_index_t*sync_gates = RTmalloc(exp_model->num_processes * sizeof(string_index_t));
    for(int i = 0; i < exp_model->num_processes; i++) {
        sync_gates[i] = SIcreate();
        for(int j = 0; j < exp_model->num_sync_rules; j++) {
            if(exp_model->sync_rules[j][i]) {
                SIput(sync_gates[i], exp_model->sync_rules[j][i]);
            }
        }
    }
    context->sync_gates = sync_gates;

    if(PINS_POR) {

        matrix_t* dna = RTmalloc(sizeof(matrix_t));
        dm_create(dna, num_groups, num_groups);
        matrix_t* mc = RTmalloc(sizeof(matrix_t));
        dm_create(mc, num_labels, num_labels);
        matrix_t* nes = RTmalloc(sizeof(matrix_t));
        dm_create(nes, num_labels, num_groups);
        matrix_t* nds = RTmalloc(sizeof(matrix_t));
        dm_create(nds, num_labels, num_groups);
        int guard_list[num_labels+1];
        for(int i = 0; i < exp_model->num_processes; i++) {
            string_index_t enabled[exp_model->processes[i].process_states];
            for(int j = 0; j < exp_model->processes[i].process_states; j++) {
                enabled[j] = SIcreate();
                exp_trans_t trans = exp_model->processes[i].transitions[j];
                for(int k = 0; trans != NULL && k < trans->num; k++) {
                    char* action = trans->label[k];
                    while(action) {
                        char* a = strpbrk(action, "\x1f");
                        if(a) {
                            a[0] = '\0';
                        }
                        exp_action_to_group_guard(context, action, i, 1, guard_list);
                        for(int l = 0; guard_list[l] != -1; l++) {
                            snprintf(buf, 20, "%d", guard_list[l]);
                            SIput(enabled[j], buf);
                        }
                        if(a) {
                            a[0] = '\x1f';
                        }
                        exp_next_label(&action);
                    }
                }
            }
            for(int j = 0; j < exp_model->processes[i].process_states; j++) {
                for(int k = 0; k < SIgetCount(enabled[j]); k++) {
                    for(int l = 0; l < SIgetCount(enabled[j]); l++) {
                        dm_set(dna, guard_to_group(context,atoi(SIget(enabled[j], k))),guard_to_group(context,atoi(SIget(enabled[j], l))));
                        dm_set(mc, atoi(SIget(enabled[j], k)), atoi(SIget(enabled[j], l)));
                    }
                }
                exp_trans_t trans = exp_model->processes[i].transitions[j];
                for(int k = 0; trans != NULL && k < trans->num; k++) {
                    char* action = trans->label[k];
                    if(action) {
                        int disabled_labels[SIgetCount(enabled[j])];
                        int disabled_count = 0;
                        for(int l = 0; l < SIgetCount(enabled[j]); l++) {
                            char* act = SIget(enabled[j], l);
                            if(SIlookup(enabled[trans->dest[k]], act) == SI_INDEX_FAILED) {
                                disabled_labels[disabled_count++] = atoi(act);
                            }
                        }
                        int enabled_labels[SIgetCount(enabled[trans->dest[k]])];
                        int enabled_count = 0;
                        for(int l = 0; l < SIgetCount(enabled[trans->dest[k]]); l++) {
                            char* act = SIget(enabled[trans->dest[k]], l);
                            if(SIlookup(enabled[j], act) == SI_INDEX_FAILED) {
                                enabled_labels[enabled_count++] = atoi(act);
                            }
                        }
                        while(action) {
                            char* a = strpbrk(action, "\x1f");
                            if(a) {
                                a[0] = '\0';
                            }
                            exp_action_to_group_guard(context, action, i, 0, guard_list);
                            for(int l = 0; guard_list[l] != -1; l++) {
                                snprintf(buf, 20, "%d", guard_list[l]);
                                for(int m = 0; m < disabled_count; m++) {
                                    dm_set(nds, disabled_labels[m], guard_list[l]);
                                }
                                for(int m = 0; m < enabled_count; m++) {
                                    dm_set(nes, enabled_labels[m], guard_list[l]);
                                }
                            }
                            if(a) {
                                a[0] = '\x1f';
                            }
                            exp_next_label(&action);
                        }
                    }
                }
            }
            for(int j = 0; j < exp_model->processes[i].process_states; j++) {
                SIdestroy(&enabled[j]);
            }
        }
        for(int i = 0; i < num_labels; i++) {
            for(int j = 0; j < num_labels; j++) {
                int proc_i = i < exp_model->num_processes ? i : (i - exp_model->num_processes) % exp_model->num_processes;
                int proc_j = j < exp_model->num_processes ? j : (j - exp_model->num_processes) % exp_model->num_processes;
                if(proc_i != proc_j) {
                    dm_set(mc, i, j);
                }
            }
        }
        GBsetDoNotAccordInfo(model, dna);
        GBsetGuardCoEnabledInfo(model, mc);
        GBsetGuardNESInfo(model, nes);
        GBsetGuardNDSInfo(model, nds);
    }
    RTfree(buf);

    GBsetContext(model,context);
}
