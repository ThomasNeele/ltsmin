#ifndef EXP_GREYBOX_H
#define EXP_GREYBOX_H


/**
\file exp-greybox.h
*/

#include <popt.h>

#include <pins-lib/pins.h>
#include <ltsmin-lib/exp-syntax.h>

extern struct poptOption exp_options[];

static exp_model_t exp_flatten_network(exp_model_t model);

int exp_next_long(model_t self, int group, int *src, TransitionCB cb, void *user_context);
/**
Load an EXP model.
*/
extern void EXPloadGreyboxModel(model_t model, const char *filename);



#endif //LTSMIN_EXP_PINS_H
