#ifndef EXP_GREYBOX_H
#define EXP_GREYBOX_H


/**
\file exp-greybox.h
*/

#include <popt.h>

#include <pins-lib/pins.h>

extern struct poptOption exp_options[];

/**
Load an EXP model.
*/
extern void EXPloadGreyboxModel(model_t model, const char *filename);



#endif //LTSMIN_EXP_PINS_H
