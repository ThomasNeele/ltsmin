#ifndef LTSMIN_EXP_PARSE_ENV_H
#define LTSMIN_EXP_PARSE_ENV_H


#include <hre-io/types.h>
#include <hre/stringindex.h>

#define ENV_BUFFER_SIZE 4096

typedef struct exp_parse_env_s *exp_parse_env_t;
typedef struct exp_model_s *exp_model_t;
typedef struct list_s *list_t;
typedef struct transition_s *transition_t;
typedef struct sync_action_number_s *sync_action_number_t;

struct exp_parse_env_s {
    void*            parser;
    void*            scanner;
    char*            filename;
    int              lineno;
    int              linepos;
    string_index_t   strings;
    exp_model_t      model;
};

struct exp_model_s {
    unsigned int     num_processes;          // 0 when this is a process, >0 when this is a network
    string_index_t   action_labels;
    string_index_t   action_gates;

    // Fields used when this a network
    unsigned int     num_sync_rules;
    char***          sync_rules;              //
    struct exp_model_s*      processes;

    // Fields used when this is a process
    unsigned int     process_states;
    unsigned int     process_transitions;
    unsigned int     initial_state;
    char***          transitions;
};

struct list_s {
    list_t prev;
    void* item;
};

struct transition_s {
    unsigned int source;
    char*        label;
    unsigned int destination;
};

struct sync_action_number_s {
    char*label;
    unsigned int number;
};

#endif //LTSMIN_EXP_PARSE_ENV_H
