#ifndef EXP_GREYBOX_H
#define EXP_GREYBOX_H


/**
\file exp-greybox.h
*/

#include <popt.h>

#include <pins-lib/pins.h>
#include <ltsmin-lib/exp-syntax.h>

extern struct poptOption exp_options[];

int determine_state_length(exp_model_t model, int *numNetworkNodes, int *numProcesses);
void network_to_array(exp_model_t model, exp_model_t *processes, int *processCounter);
//int getInitialState(exp_model_t model, int *init_state, int offset_bits);
void collect_edge_labels(exp_model_t model, int *processCounter, int *labelCount, char ***names);

int exp_next_long(model_t self, int group, int *src, TransitionCB cb, void *user_context);
/**
Load an EXP model.
*/
extern void EXPloadGreyboxModel(model_t model, const char *filename);



#endif //LTSMIN_EXP_PINS_H
