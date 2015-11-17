#ifndef LTSMIN_EXP_PARSE_ENV_H
#define LTSMIN_EXP_PARSE_ENV_H


#include <hre-io/types.h>
#include <hre/stringindex.h>

#define ENV_BUFFER_SIZE 4096

typedef struct exp_parse_env_s *exp_parse_env_t;
typedef struct exp_model_s *exp_model_t;
typedef struct list_s *list_t;
typedef struct transition_s *transition_t;

struct exp_parse_env_s {
    stream_t         input;
    void*            parser;
    int              lineno;
    int              linepos;
    string_index_t   strings;
    exp_model_t      model;
    char             buffer[ENV_BUFFER_SIZE];
};

struct exp_model_s {
    unsigned int     num_processes;          // 0 when this is a process, >0 when this is a network

    // Fields used when this a network
    unsigned int     num_sync_rules;
    char***          sync_rules;              //
    exp_model_t*     processes;

    // Fields used when this is a process
    unsigned int     process_states;
    unsigned int     process_transitions;
    unsigned int     initial_state;
    string_index_t** transitions;
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

#endif //LTSMIN_EXP_PARSE_ENV_H
