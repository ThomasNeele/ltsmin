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
    int num_sync_rules = model->num_sync_rules;
    int num_processes = 0;
    for(int i = 0; i < model->num_processes; i++) {
        processes[i] = flatten_network(&model->processes[i]);
        num_processes += processes[i]->num_processes;
        num_sync_rules += processes[i]->num_sync_rules;
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

    char*** sync_rules[num_sync_rules][num_processes];
    int rule_counter = 0;
    for(int i = 0; i < model->num_sync_rules; i++) {
        int action_counter = 0;
        for(int j = 0; j < model->num_processes; j++) {
            int subaction_is_syncresult = 0;
            for(int k = 0; k < model->processes[j].num_sync_rules; k++) {

            }
        }
    }
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
    exp_model_t exp_model = (exp_model_t) user_context;
    transition_info_t ti = GB_TI(NULL, group);

//    cb(user_context, ti, dst, NULL);
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

    lts_type_t ltstype = lts_type_create();
    int numNetworkNodes = 0;
    int numProcesses = 0;
    determine_state_length(exp_model, &numNetworkNodes, &numProcesses);
    lts_type_set_state_length(ltstype, numProcesses);

    int processCounter = 0;
    exp_model_t* processes = RTmalloc(sizeof(exp_model_t) * numNetworkNodes);
    network_to_array(exp_model, processes, &processCounter);

    int* labelCount = RTmalloc(sizeof(int) * numNetworkNodes);
    char*** labelNames = RTmalloc(sizeof(char**) * numNetworkNodes);
    processCounter = 0;
    collect_edge_labels(exp_model, &processCounter, labelCount, labelNames);
    int totalNumLabels = 0;
    for(int i = 0; i < numNetworkNodes; i++) {
        totalNumLabels += labelCount[i];
    }
    lts_type_set_edge_label_count(ltstype, totalNumLabels);

    int* init_state = RTmalloc(sizeof(int) * numProcesses);
    matrix_t* p_dm = (matrix_t*)RTmalloc(sizeof(matrix_t));
    dm_create(p_dm, numNetworkNodes, numProcesses);

    int is_new = 1;
    int stringType = lts_type_put_type(ltstype, "string", LTStypeChunk, &is_new);
    processCounter = 0;
    int labelCounter = 0;
    for(int i = 0; i < numNetworkNodes; i++) {
        if (processes[i]->num_processes == 0) {
            init_state[processCounter] = processes[i]->initial_state;

            char *buf = RTmalloc(sizeof(char) * 20);
            snprintf(buf, 19, "[0,%d]", processes[i]->process_states - 1);

            is_new = 1;
            int typeno = lts_type_put_type(ltstype, buf, LTStypeRange, &is_new);
            if (is_new) {
                lts_type_set_range(ltstype, typeno, 0, processes[i]->process_states - 1);
            }
            lts_type_set_state_typeno(ltstype, processCounter, typeno);
            buf = RTmalloc(sizeof(char) * 20);
            snprintf(buf, 19, "proc%d", i);
            lts_type_set_state_name(ltstype, processCounter, buf);

            // Transitions of this process are dependent on this process
            dm_set(p_dm, i, processCounter);

            processCounter++;
        } else {
            // Transitions of nodes can influence multiple processes
            for(int j = 0; j < numProcesses; j++) {
                dm_set(p_dm, i, j);
            }
        }

        for (int j = 0; j < labelCount[i]; j++) {
            lts_type_set_edge_label_typeno(ltstype, labelCounter, stringType);
            lts_type_set_edge_label_name(ltstype, labelCounter, labelNames[i][j]);
            labelCounter++;
        }
    }

    GBsetContext(model,exp_model);
    GBsetLTStype(model,ltstype);

    GBsetDMInfo(model, p_dm);
    GBsetNextStateLong(model, exp_next_long);

    GBsetInitialState(model, init_state);
}
